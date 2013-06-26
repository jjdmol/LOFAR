//# MeasurementAIPS.cc: Specialisation of Measurement that understands the
//# AIPS++ measurement set format.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBSKernel/MeasurementAIPS.h>
#include <BBSKernel/Exceptions.h>

#include <Common/Timer.h>
#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasTable.h>
#include <measures/Measures/MeasConvert.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ExprNodeSet.h>
#include <tables/Tables/TableIter.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TiledColumnStMan.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>

#include <casa/BasicMath/Math.h>
#include <casa/Utilities/GenSort.h>

#include <Common/lofar_iomanip.h>
#include <Common/Version.h>
#include <BBSKernel/Package__Version.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

namespace
{
    bool hasColumn(const Table &table, const string &column);
    bool hasSubTable(const Table &table, const string &name);
    Table getSubTable(const Table &table, const string &name);

    Station::Ptr readStation(const Table &table, unsigned int id,
        const string &name, const MPosition &position);
}

MeasurementAIPS::MeasurementAIPS(const string &filename,
    unsigned int idObservation, unsigned int idField,
    unsigned int idDataDescription)
    :   itsMS(filename),
        itsFreqAxisReversed(false),
        itsIdObservation(idObservation),
        itsIdField(idField),
        itsIdDataDescription(idDataDescription)
{
    // Get information about the telescope (instrument).
    itsInstrument = readInstrument(itsMS, itsIdObservation);

    // Get the reference directions for the selected field (i.e. phase center,
    // delay center, tile delay center).
    initReferenceDirections();

    // Select a single measurement from the measurement set.
    // TODO: At the moment we use (OBSERVATION_ID, FIELD_ID, DATA_DESC_ID) as
    // the key to select a single measurement. This may not be strict enough,
    // i.e. different measurements may still have the same key.
    itsMainTableView =
        itsMS(itsMS.col("OBSERVATION_ID") == static_cast<Int>(idObservation)
        && itsMS.col("FIELD_ID") == static_cast<Int>(idField)
        && itsMS.col("DATA_DESC_ID") == static_cast<Int>(idDataDescription));

    // Get the dimensions (time, frequency, baselines, correlations) of the
    // measurement.
    initDimensions();

    LOG_DEBUG_STR("Measurement " << idObservation << " contains "
        << itsMainTableView.nrow() << " row(s).");
    LOG_DEBUG_STR("Measurement dimensions:\n" << ((Measurement*) this)->dims());
}

VisDimensions MeasurementAIPS::dims(const VisSelection &selection) const
{
    return getDimensionsImpl(getVisSelection(itsMainTableView, selection),
        getCellSlicer(selection));
}

VisBuffer::Ptr MeasurementAIPS::read(const VisSelection &selection,
    const string &column, bool readCovariance, bool readFlags) const
{
    NSTimer readTimer, copyTimer;

    // Find the column with covariance information associated with the specified
    // column.
    string columnCov = getLinkedCovarianceColumn(column,
        hasColumn(itsMS, "WEIGHT_SPECTRUM") ? "WEIGHT_SPECTRUM" : "WEIGHT");

    // Create cell slicers for array columns.
    Slicer slicer = getCellSlicer(selection);
    Slicer slicerCov = getCovarianceSlicer(selection, columnCov);

    // Get a view on the selection.
    Table tab_selection = getVisSelection(itsMainTableView, selection);

    // Get the dimensions of the visibility data that matches the selection.
    VisDimensions dims = getDimensionsImpl(tab_selection, slicer);

    // Allocate a buffer for visibility data.
    VisBuffer::Ptr buffer(new VisBuffer(dims, readCovariance, readFlags));
    buffer->setInstrument(itsInstrument);
    buffer->setReferenceFreq(itsReferenceFreq);
    buffer->setPhaseReference(getColumnPhaseReference(column));
    buffer->setDelayReference(itsDelayReference);
    buffer->setTileReference(itsTileReference);

    if(dims.empty())
    {
        return buffer;
    }

    if(readFlags)
    {
        // Initialize all flags to 1, which effectively means all samples are
        // flagged. This way, if certain data is missing in the measurement set
        // the corresponding samples in the buffer will be uninitialized but
        // also flagged.
        buffer->flagsSet(1);
    }

    LOG_DEBUG_STR("Reading visibilities from column: " << column);
    LOG_DEBUG_STR("Reading noise covariance from linked column: " << columnCov);

    const size_t nFreq = dims.nFreq();
    const size_t nCorrelations = dims.nCorrelations();

    size_t tslot = 0;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < dims.nTime(), "Timeslot out of range.");

        // Get next timeslot.
        Table tab_tslot = tslotIterator.table();
        const size_t nRows = tab_tslot.nrow();

        // Read data from the MS.
        readTimer.start();
        ROScalarColumn<Int> c_antenna1(tab_tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tab_tslot, "ANTENNA2");
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        ASSERT(aips_antenna1.shape().isEqual(IPosition(1, nRows)));
        ASSERT(aips_antenna2.shape().isEqual(IPosition(1, nRows)));

        ROArrayColumn<Complex> c_data(tab_tslot, column);
        Cube<Complex> aips_data = c_data.getColumn(slicer);
        ASSERT(aips_data.shape().isEqual(IPosition(3, nCorrelations, nFreq,
            nRows)));
        ASSERT(aips_data.contiguousStorage());

        Array<Float> aips_covariance;
        if(readCovariance)
        {
            ROArrayColumn<Float> c_covariance(tab_tslot, columnCov);
            Array<Float> aips_cov_tmp = c_covariance.getColumn(slicerCov);

            // Make sure covariance information has the right shape and convert
            // from weight to covariance if necessary.
            aips_covariance.reference(reformatCovarianceArray(aips_cov_tmp,
                nCorrelations, nFreq, nRows));

            ASSERT(aips_covariance.shape().isEqual(IPosition(4, nCorrelations,
                nCorrelations, nFreq, nRows)));
            ASSERT(aips_covariance.contiguousStorage());
        }

        Vector<Bool> aips_flag_row;
        Cube<Bool> aips_flag;
        if(readFlags)
        {
            ROScalarColumn<Bool> c_flag_row(tab_tslot, "FLAG_ROW");
            aips_flag_row.reference(c_flag_row.getColumn());
            ASSERT(aips_flag_row.shape().isEqual(IPosition(1, nRows)));

            ROArrayColumn<Bool> c_flag(tab_tslot, "FLAG");
            aips_flag.reference(c_flag.getColumn(slicer));
            ASSERT(aips_flag.shape().isEqual(IPosition(3, nCorrelations, nFreq,
                nRows)));
            ASSERT(aips_flag.contiguousStorage());
        }
        readTimer.stop();

        // Use multi_array_ref to ease transposing the data to per baseline
        // timeseries. This also accounts for reversed channels if necessary.
        bool ascending[] = {true, !itsFreqAxisReversed, true};
        boost::const_multi_array_ref<Complex, 3>::size_type order_data[] =
            {2, 1, 0};
        boost::const_multi_array_ref<Complex, 3> samples(aips_data.data(),
            boost::extents[nRows][nFreq][nCorrelations],
            boost::general_storage_order<3>(order_data, ascending));

        bool ascending_covariance[] = {true, !itsFreqAxisReversed, true, true};
        boost::const_multi_array_ref<Float, 4>::size_type order_covariance[] =
            {3, 2, 1, 0};
        const Float *ptrCovariance = readCovariance ? aips_covariance.data()
            : static_cast<const Float*>(0);
        boost::const_multi_array_ref<Float, 4> covariance(ptrCovariance,
            boost::extents[nRows][nFreq][nCorrelations][nCorrelations],
            boost::general_storage_order<4>(order_covariance,
            ascending_covariance));

        boost::const_multi_array_ref<Bool, 3>::size_type order_flag[] =
            {2, 1, 0};
        const Bool *ptrFlags = readFlags ? aips_flag.data()
            : static_cast<const Bool*>(0);
        boost::const_multi_array_ref<Bool, 3> flags(ptrFlags,
            boost::extents[nRows][nFreq][nCorrelations],
            boost::general_storage_order<3>(order_flag, ascending));

        copyTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            baseline_t baseline(aips_antenna1[i], aips_antenna2[i]);

            // Look up baseline index.
            size_t basel = dims.baselines().index(baseline);
            ASSERT(basel < dims.nBaselines());

            // Copy visibilities.
            buffer->samples[basel][tslot] = samples[i];

            if(readCovariance)
            {
                // Copy covariance.
                buffer->covariance[basel][tslot] = covariance[i];
            }

            if(readFlags)
            {
                // If the entire row is flagged do nothing (all sample flags
                // have been initialized to 1 above). Otherwise, copy the flags
                // read from disk into the buffer.
                if(!aips_flag_row(i))
                {
                    // Copy flags.
                    buffer->flags[basel][tslot] = flags[i];
                }
            }
        }
        copyTimer.stop();

        // Proceed to the next timeslot.
        ++tslot;
        ++tslotIterator;
    }

    LOG_DEBUG_STR("Read time: " << readTimer);
    LOG_DEBUG_STR("Copy time: " << copyTimer);

    return buffer;
}

void MeasurementAIPS::write(VisBuffer::Ptr buffer,
    const VisSelection &selection, const string &column, bool writeCovariance,
    bool writeFlags, flag_t flagMask)
{
    ASSERT(!writeFlags || buffer->hasFlags());
    ASSERT(!writeCovariance || buffer->hasCovariance());

    NSTimer readTimer, writeTimer;

    // Reopen MS for writing.
    itsMS.reopenRW();
    ASSERT(itsMS.isWritable());

    // Add visbility column if it does not exist.
    if(!hasColumn(itsMS, column))
    {
        LOG_INFO_STR("Creating visibility column: " << column);
        createVisibilityColumn(column);
    }

    // Determine the name of the covariance column to write to. Writing
    // covariance information for the DATA column is currently not supported.
    string columnCov = getLinkedCovarianceColumn(column, "LOFAR_" + column
        + "_COVARIANCE");

    if(writeCovariance)
    {
        // Store a link to the covariance column as a column keyword of the
        // visibility column.
        setLinkedCovarianceColumn(column, columnCov);

        // Add covariance column if it does not exist.
        if(!hasColumn(itsMS, columnCov))
        {
            LOG_INFO_STR("Creating covariance column: " << columnCov);
            createCovarianceColumn(columnCov);
        }
    }

    // If the buffer to write is empty, return immediately.
    if(buffer->dims().empty())
    {
        return;
    }

    LOG_DEBUG_STR("Writing visibilities to column: " << column);
    if(writeCovariance)
    {
        LOG_DEBUG_STR("Writing noise covariance to linked column: "
            << columnCov);
    }

    // Get a view on the selection.
    Table tab_selection = getVisSelection(itsMainTableView, selection);

    // Create cell slicers for array columns.
    Interval<size_t> range = getChannelRange(selection);
    Slicer slicer(IPosition(2, 0, range.start),
        IPosition(2, nCorrelations() - 1, range.end),
        Slicer::endIsLast);
    Slicer slicerCov(IPosition(3, 0, 0, range.start),
        IPosition(3, nCorrelations() - 1, nCorrelations() - 1, range.end),
        Slicer::endIsLast);

    // Allocate temporary buffers to be able to reverse frequency channels.
    // TODO: Some performance can be gained by creating a specialized
    // implementation for the case where the channels are in ascending order to
    // avoid the extra copy.
    bool ascending[] = {!itsFreqAxisReversed, true};
    boost::multi_array<Bool, 2>::size_type order_flag[] = {1, 0};
    boost::multi_array<Bool, 2>
        flagBuffer(boost::extents[buffer->nFreq()][buffer->nCorrelations()],
            boost::general_storage_order<2>(order_flag, ascending));

    boost::multi_array<Complex, 2>::size_type order_data[] = {1, 0};
    boost::multi_array<Complex, 2>
        sampleBuffer(boost::extents[buffer->nFreq()][buffer->nCorrelations()],
            boost::general_storage_order<2>(order_data, ascending));

    bool ascending_covariance[] = {!itsFreqAxisReversed, true, true};
    boost::multi_array<Float, 3>::size_type order_covariance[] = {2, 1, 0};
    boost::multi_array<Float, 3>
        covarianceBuffer(boost::extents[buffer->nFreq()]
            [buffer->nCorrelations()][buffer->nCorrelations()],
            boost::general_storage_order<3>(order_covariance,
                ascending_covariance));

    size_t tslot = 0;
    bool mismatch = false;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < buffer->nTime(), "Timeslot out of range.");

        // Extract next timeslot.
        Table tab_tslot = tslotIterator.table();
        size_t nRows = tab_tslot.nrow();

        // TODO: Should use TIME_CENTROID here (centroid of exposure)? NB. The
        // TIME_CENTROID may be different for each baseline!
        ROScalarColumn<Double> c_time(tab_tslot, "TIME");
        ROScalarColumn<Int> c_antenna1(tab_tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tab_tslot, "ANTENNA2");

        // Read meta data.
        readTimer.start();
        Vector<Double> aips_time = c_time.getColumn();
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        readTimer.stop();

        ScalarColumn<Bool> c_flag_row;
        ArrayColumn<Bool> c_flag;
        if(writeFlags)
        {
            // Open flag columns for writing.
            c_flag_row.attach(tab_tslot, "FLAG_ROW");
            c_flag.attach(tab_tslot, "FLAG");
        }

        // Open data column for writing.
        ArrayColumn<Complex> c_data(tab_tslot, column);

        ArrayColumn<Float> c_covariance;
        if(writeCovariance)
        {
            // Open covariance column for writing.
            c_covariance.attach(tab_tslot, columnCov);
        }

        mismatch = mismatch
            && (buffer->grid()[TIME]->center(tslot) == aips_time(0));

        writeTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            baseline_t baseline(aips_antenna1[i], aips_antenna2[i]);

            // Get time sequence for this baseline.
            size_t basel = buffer->baselines().index(baseline);
            ASSERT(basel < buffer->nBaselines());

            if(writeFlags)
            {
                // Copy flags and-ed with flagMask. Reversal of the frequency
                // axis is taken care of by the storage order assigned to
                // flagBuffer if required (see above). Also, determine if the
                // entire row should be flagged (if all sample flags equal
                // true).
                bool flagRow = true;
                for(size_t j = 0; j < buffer->nFreq(); ++j)
                {
                    for(size_t k = 0; k < buffer->nCorrelations(); ++k)
                    {
                        flagBuffer[j][k] =
                            buffer->flags[basel][tslot][j][k] & flagMask;
                        flagRow = flagRow && flagBuffer[j][k];
                    }
                }

                // Write row flag.
                c_flag_row.put(i, flagRow);

                // Write visibility flags.
                Array<Bool> flags(IPosition(2, buffer->nCorrelations(),
                    buffer->nFreq()), flagBuffer.data(), SHARE);
                c_flag.putSlice(i, slicer, flags);
            }

            // Copy visibilities and optionally reverse (handled via the storage
            // order assigned to sampleBuffer, see above).
            sampleBuffer = buffer->samples[basel][tslot];

            // Write visibilities.
            Array<Complex> samples(IPosition(2, buffer->nCorrelations(),
                buffer->nFreq()), sampleBuffer.data(), SHARE);
            c_data.putSlice(i, slicer, samples);

            if(writeCovariance)
            {
                // Copy covariance and optionally reverse (handled via the
                // storage order assigned to covarianceBuffer, see above).
                covarianceBuffer = buffer->covariance[basel][tslot];

                // Write covariance.
                Array<Float> covariance(IPosition(3, buffer->nCorrelations(),
                    buffer->nCorrelations(), buffer->nFreq()),
                    covarianceBuffer.data(), SHARE);

                c_covariance.putSlice(i, slicerCov, covariance);
            }
        }
        writeTimer.stop();

        ++tslot;
        ++tslotIterator;
    }

    // Flush data to disk. This is where most of the time is spent.
    writeTimer.start();
    tab_selection.flush(true, true);
    writeTimer.stop();

    if(mismatch)
    {
        LOG_WARN_STR("Time mismatch(es) detected while writing data.");
    }

    LOG_DEBUG_STR("Read time (meta data): " << readTimer);
    LOG_DEBUG_STR("Write time: " << writeTimer);
}

void MeasurementAIPS::writeHistory(const ParameterSet &parset) const
{
    Table tab_history = getSubTable(itsMS, "HISTORY");
    tab_history.reopenRW();

    ScalarColumn<double> c_time(tab_history, "TIME");
    ScalarColumn<int> c_observationId(tab_history, "OBSERVATION_ID");
    ScalarColumn<String> c_message(tab_history, "MESSAGE");
    ScalarColumn<String> c_application(tab_history, "APPLICATION");
    ScalarColumn<String> c_priority(tab_history, "PRIORITY");
    ScalarColumn<String> c_origin(tab_history, "ORIGIN");
    ArrayColumn<String> c_appParams(tab_history, "APP_PARAMS");
    ArrayColumn<String> c_cliCommand(tab_history, "CLI_COMMAND");

    Vector<String> appParams, cliCommand;
    if(c_appParams.columnDesc().isFixedShape())
    {
        // Some WSRT MS have an APP_PARAMS and CLI_COMMAND column with fixed shape.
        // For them, put all params in a single vector element (with newlines).
        // Check implicit assumptions.
        ASSERT(c_appParams.columnDesc().shape().isEqual(IPosition(1, 1)));
        ASSERT(c_cliCommand.columnDesc().isFixedShape());
        ASSERT(c_cliCommand.columnDesc().shape().isEqual(IPosition(1, 1)));

        appParams.resize(1);
        cliCommand.resize(1);
        parset.writeBuffer(appParams(0));
    }
    else
    {
        appParams.resize(parset.size());

        Array<String>::contiter out = appParams.cbegin();
        for(ParameterSet::const_iterator it = parset.begin();
            it != parset.end(); ++it, ++out)
        {
            *out = it->first + '=' + it->second.get();
        }
    }

    unsigned int index = tab_history.nrow();
    tab_history.addRow();
    c_time.put(index, Time().modifiedJulianDay() * 24.0 * 3600.0);
    c_observationId.put(index, itsIdObservation);
    c_message.put(index, "parameters");
    c_application.put(index, "BBS");
    c_priority.put(index, "NORMAL");
    c_origin.put(index, Version::getInfo<BBSKernelVersion>("BBS", "other"));
    c_appParams.put(index, appParams);
    c_cliCommand.put(index, cliCommand);

    // Flush changes to disk to minimize the chance on index corruption.
    tab_history.flush(true, true);
}

BaselineMask MeasurementAIPS::asMask(const string &filter) const
{
    // Find all unique baselines.
    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "ANTENNA2";
    Table tab_baselines = itsMainTableView.sort(sortColumns, Sort::Ascending,
        Sort::HeapSort | Sort::NoDuplicates);

    // Select all baselines that match the given pattern.
    Table tab_selection(getBaselineSelection(tab_baselines, filter));

    // Fetch the selected baselines.
    Vector<Int> antenna1 =
        ROScalarColumn<Int>(tab_selection, "ANTENNA1").getColumn();
    Vector<Int> antenna2 =
        ROScalarColumn<Int>(tab_selection, "ANTENNA2").getColumn();

    // Construct a BaselineMask from the selected baseline.
    BaselineMask mask;
    for(size_t i = 0; i < antenna1.size(); ++i)
    {
        mask.set(antenna1[i], antenna2[i]);
        mask.set(antenna2[i], antenna1[i]);
    }

    return mask;
}

void MeasurementAIPS::initReferenceDirections()
{
    itsPhaseReference = MDirection::Convert(readPhaseReference(itsMS,
        itsIdField), MDirection::J2000)();
    itsDelayReference = MDirection::Convert(readDelayReference(itsMS,
        itsIdField), MDirection::J2000)();
    itsTileReference = MDirection::Convert(readTileReference(itsMS,
        itsIdField), MDirection::J2000)();
}

void MeasurementAIPS::initDimensions()
{
    // Read polarization id and spectral window id.
    ROMSDataDescColumns desc(itsMS.dataDescription());
    ASSERT(desc.nrow() > itsIdDataDescription);
    ASSERT(!desc.flagRow()(itsIdDataDescription));

    const unsigned int idPolarization =
        desc.polarizationId()(itsIdDataDescription);
    const unsigned int idWindow = desc.spectralWindowId()(itsIdDataDescription);

    // Get spectral information.
    ROMSSpWindowColumns window(itsMS.spectralWindow());
    ASSERT(window.nrow() > idWindow);
    ASSERT(!window.flagRow()(idWindow));

    itsReferenceFreq = window.refFrequency()(idWindow);

    const unsigned int nFreq = window.numChan()(idWindow);
    Vector<Double> frequency = window.chanFreq()(idWindow);
    Vector<Double> width = window.chanWidth()(idWindow);

    ASSERT(frequency.nelements() == nFreq);
    ASSERT(width.nelements() == nFreq);
    // TODO: Technically, checking for equal channel widths is not enough,
    // because there could still be gaps between channels even though the
    // widths are all equal (this is not prevented by the MS 2.0 standard).
    ASSERTSTR(allEQ(width, width(0)),
        "Channels width is not the same for all channels. This is not supported"
        " yet.");

    double lower = frequency(0) - 0.5 * width(0);
    double upper = frequency(nFreq - 1) + 0.5 * width(nFreq - 1);
    if(frequency(0) > frequency(nFreq - 1))
    {
        LOG_INFO_STR("Channels are in reverse order.");

        // Correct upper and lower boundary.
        lower = frequency(nFreq - 1) - 0.5 * width(nFreq - 1);
        upper = frequency(0) + 0.5 * width(0);
        itsFreqAxisReversed = true;
    }

    // Construct frequency axis.
    Axis::ShPtr freqAxis(new RegularAxis(lower, (upper - lower) / nFreq,
        nFreq));

    // Extract time axis based on TIME column (mid-point of integration
    // interval).
    // TODO: Should use TIME_CENTROID here (centroid of exposure)? NB. UVW is
    // given of the TIME_CENTROID, not for TIME! NB. The TIME_CENTROID may be
    // different for each baseline!

    // Find all unique integration cells.
    Table tab_sorted = itsMainTableView.sort("TIME", Sort::Ascending,
        Sort::HeapSort | Sort::NoDuplicates);

    // Read TIME and INTERVAL column.
    ROScalarColumn<Double> c_time(tab_sorted, "TIME");
    ROScalarColumn<Double> c_interval(tab_sorted, "INTERVAL");
    Vector<Double> time = c_time.getColumn();
    Vector<Double> interval = c_interval.getColumn();

    // Convert to vector<double>.
    vector<double> timeCopy;
    vector<double> intervalCopy;
    time.tovector(timeCopy);
    interval.tovector(intervalCopy);

    // Construct time axis.
    Axis::ShPtr timeAxis(new OrderedAxis(timeCopy, intervalCopy));
    itsDims.setGrid(Grid(freqAxis, timeAxis));

    // Find all unique baselines.
    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "ANTENNA2";
    Table tab_baselines = itsMainTableView.sort(sortColumns, Sort::Ascending,
        Sort::HeapSort | Sort::NoDuplicates);
    const unsigned int nBaselines = tab_baselines.nrow();

    // Initialize baseline axis.
    ROScalarColumn<Int> c_antenna1(tab_baselines, "ANTENNA1");
    ROScalarColumn<Int> c_antenna2(tab_baselines, "ANTENNA2");
    Vector<Int> antenna1 = c_antenna1.getColumn();
    Vector<Int> antenna2 = c_antenna2.getColumn();

    BaselineSeq baselines;
    for(unsigned int i = 0; i < nBaselines; ++i)
    {
        baselines.append(baseline_t(antenna1[i], antenna2[i]));
    }
    itsDims.setBaselines(baselines);

    // Initialize correlation axis.
    ROMSPolarizationColumns polarization(itsMS.polarization());
    ASSERT(polarization.nrow() > idPolarization);
    ASSERT(!polarization.flagRow()(idPolarization));

    Vector<Int> corrType = polarization.corrType()(idPolarization);
    const unsigned int nCorrelations = corrType.nelements();

    CorrelationSeq correlations;
    for(unsigned int i = 0; i < nCorrelations; ++i)
    {
        Correlation::Type cr =
            Correlation::asCorrelation(Stokes::name(Stokes::type(corrType(i))));
        correlations.append(cr);
    }

    ASSERTSTR(correlations.size() == nCorrelations, "MS contains duplicate"
        " correlations.");

    itsDims.setCorrelations(correlations);
}

void MeasurementAIPS::createVisibilityColumn(const string &name)
{
    // Added column should get the same shape as the other data columns in
    // the MS.
    ArrayColumnDesc<Complex> columnDescriptor(name, IPosition(2,
        nCorrelations(), nFreq()), ColumnDesc::FixedShape);

    // Create storage manager. Tile size specification taken from the
    // MSCreate class in the CEP/MS package.
    TiledColumnStMan storageManager("Tiled_" + name, IPosition(3,
        nCorrelations(), nFreq(), std::max(size_t(1), 4096 / nFreq())));

    itsMS.addColumn(columnDescriptor, storageManager);
    itsMS.flush();

    // Re-create the view on the selected measurement. Otherwise,
    // itsMainTableView has no knowledge of the added column.
    //
    // TODO: At the moment we use (OBSERVATION_ID, FIELD_ID, DATA_DESC_ID)
    // as the key to select a single measurement. This may not be strict
    // enough, i.e. different measurements may still have the same key.
    itsMainTableView =
        itsMS(itsMS.col("OBSERVATION_ID")
            == static_cast<Int>(itsIdObservation)
        && itsMS.col("FIELD_ID")
            == static_cast<Int>(itsIdField)
        && itsMS.col("DATA_DESC_ID")
            == static_cast<Int>(itsIdDataDescription));
}

void MeasurementAIPS::createCovarianceColumn(const string &name)
{
    // Store dimensions in local variables to improve code readability. The
    // dimensions are converted to int because that is what is used by
    // casa::Array.
    const int nFreq = this->nFreq();
    const int nCorrelations = this->nCorrelations();

    // Determine the shape of the column. The shape is fixed, i.e. it is
    // constrained to be the same for all the rows of the column.
    ArrayColumnDesc<Float> columnDescriptor(name, IPosition(3, nCorrelations,
        nCorrelations, nFreq), ColumnDesc::FixedShape);

    // Create storage manager. Tile size specification taken from the MSCreate
    // class in the CEP/MS package.
    TiledColumnStMan storageManager("Tiled_" + name, IPosition(4,
        nCorrelations, nCorrelations, nFreq, std::max(1, 4096 / nFreq)));

    // Add the column.
    itsMS.addColumn(columnDescriptor, storageManager);

    // Figure out which column to use as input.
    bool hasSpectrum = hasColumn(itsMS, "WEIGHT_SPECTRUM");
    string weightColumn = hasSpectrum ? "WEIGHT_SPECTRUM" : "WEIGHT";

    // Initialize the covariance column using the weight (assumed to equal one
    // over the variance) per channel (WEIGHT_SPECTRUM) if available.
    TableIterator tslotIterator(itsMS, "TIME");
    while(!tslotIterator.pastEnd())
    {
        // Get a view on a single time slot (all rows with an identical TIME
        // value).
        Table tab_tslot = tslotIterator.table();
        const int nRows = tab_tslot.nrow();

        // Declare the columns that are going to be used.
        ROArrayColumn<Float> c_weight(tab_tslot, weightColumn);
        ArrayColumn<Float> c_covariance(tab_tslot, name);

        // Read the weights (one over the variance of the noise).
        Array<Float> weight = c_weight.getColumn();

        // Allocate a buffer for the covariance information of this time slot.
        // NB. It is not guaranteed that nRows is always the same for all time
        // slots. Therefore this buffer is allocated here instead of outside the
        // enclosing while loop.
        Array<Float> covariance(IPosition(4, nCorrelations, nCorrelations,
            nFreq, nRows), 0.0);

        if(hasSpectrum)
        {
            // Shape should be nCorrelations x nFreq x nRows.
            ASSERT(weight.shape().isEqual(IPosition(3, nCorrelations, nFreq,
                nRows)));

            // Initialize the diagonal of the noise covariance matrices using
            // the weights read from disk.
            IPosition idxOut(4, 0);
            IPosition idxIn(3, 0);
            for(idxOut[3] = 0, idxIn[2] = 0;
                idxOut[3] < nRows;
                ++idxOut[3], ++idxIn[2])
            {
                for(idxOut[2] = 0, idxIn[1] = 0;
                    idxOut[2] < nFreq;
                    ++idxOut[2], ++idxIn[1])
                {
                    for(idxOut[0] = 0, idxOut[1] = 0, idxIn[0] = 0;
                        idxOut[0] < nCorrelations;
                        ++idxOut[0], ++idxOut[1], ++idxIn[0])
                    {
                        covariance(idxOut) = 1.0 / weight(idxIn);
                    }
                }
            }
        }
        else
        {
            // Shape should be nCorrelations x nRows.
            ASSERT(weight.shape().isEqual(IPosition(2, nCorrelations, nRows)));

            // Initialize the diagonal of the noise covariance matrices using
            // the weights read from disk.
            IPosition idxOut(4, 0);
            IPosition idxIn(2, 0);
            for(idxOut[3] = 0, idxIn[1] = 0;
                idxOut[3] < nRows;
                ++idxOut[3], ++idxIn[1])
            {
                for(idxOut[2] = 0; idxOut[2] < nFreq; ++idxOut[2])
                {
                    for(idxOut[0] = 0, idxOut[1] = 0, idxIn[0] = 0;
                        idxOut[0] < nCorrelations;
                        ++idxOut[0], ++idxOut[1], ++idxIn[0])
                    {
                        covariance(idxOut) = 1.0 / weight(idxIn);
                    }
                }
            }
        }

        // Write the covariance matrices to disk.
        c_covariance.putColumn(covariance);

        // Move to the next timeslot.
        ++tslotIterator;
    }

    // Flush buffered information to disk.
    itsMS.flush();

    // Re-create the view on the selected measurement. Otherwise,
    // itsMainTableView has no knowledge of the added column.
    //
    // TODO: At the moment we use (OBSERVATION_ID, FIELD_ID, DATA_DESC_ID)
    // as the key to select a single measurement. This may not be strict
    // enough, i.e. different measurements may still have the same key.
    itsMainTableView =
        itsMS(itsMS.col("OBSERVATION_ID")
            == static_cast<Int>(itsIdObservation)
        && itsMS.col("FIELD_ID")
            == static_cast<Int>(itsIdField)
        && itsMS.col("DATA_DESC_ID")
            == static_cast<Int>(itsIdDataDescription));
}

MDirection MeasurementAIPS::getColumnPhaseReference(const string &column) const
{
    static const String keyword = "LOFAR_DIRECTION";

    ROTableColumn tableColumn(itsMS, column);
    const TableRecord &keywordSet = tableColumn.keywordSet();
    if(keywordSet.isDefined(keyword))
    {
        try
        {
            String error;
            MeasureHolder mHolder;
            if(!mHolder.fromRecord(error, keywordSet.asRecord(keyword)))
            {
                THROW(BBSKernelException, "Error reading keyword: " << keyword
                    << " for column: " << column);
            }

            return mHolder.asMDirection();
        }
        catch(AipsError &e)
        {
            THROW(BBSKernelException, "Error reading keyword: " << keyword
                << " for column: " << column);
        }
    }

    return itsPhaseReference;
}

// NOTE: OPTIMIZATION OPPORTUNITY: Cache implementation specific selection
// within a specialization of VisSelection or VisBuffer.
Table MeasurementAIPS::getVisSelection(Table table,
    const VisSelection &selection) const
{
    // Apply baseline selection if required.
    if(selection.isSet(VisSelection::BASELINE_FILTER))
    {
        table = getBaselineSelection(table, selection.getBaselineFilter());
    }

    // Alas, table(TableExprNode(true)) raises an exception instead of selecting
    // everything. Fortunately, table(TableExprNode()) does select everything
    // and even though a default constructed TableExprNode is "null", using it
    // as an operand to && seems to work as if it equates to true.
    TableExprNode filter;

    Interval<double> timeRange = selection.getTimeRange();
    if(selection.isSet(VisSelection::TIME_START))
    {
        filter = filter && table.col("TIME") >= timeRange.start;
    }

    if(selection.isSet(VisSelection::TIME_END))
    {
        filter = filter && table.col("TIME") <= timeRange.end;
    }

    if(selection.isSet(VisSelection::CORRELATION_MASK))
    {
        THROW(BBSKernelException, "Reading a subset of the available"
            " correlations is not yet implemented.");
    }

    return table(filter);
}

Table MeasurementAIPS::getBaselineSelection(const Table &table,
    const string &pattern) const
{
    try
    {
        // Create a MeasurementSet instance from the table of unique baselines.
        // This is required because MSSelection::toTableExprNode only operates
        // on MeasurementSets.
        MeasurementSet ms(table);

        MSSelection selection;
        if(!selection.setAntennaExpr(pattern))
        {
            THROW(BBSKernelException, "MSSelection::setAntennaExpr() failed.");
        }

        return ms(selection.toTableExprNode(&ms));
    }
    catch(AipsError &e)
    {
        THROW(BBSKernelException, "Error in baseline selection [" << e.what()
            << "]");
    }
}

BaselineMask MeasurementAIPS::getBaselineMask(const VisSelection &selection)
    const
{
    if(selection.isSet(VisSelection::BASELINE_FILTER))
    {
        return asMask(selection.getBaselineFilter());
    }

    // By default, select all baselines (cross _and_ auto-correlations).
    return asMask("*&&");
}

// NOTE: OPTIMIZATION OPPORTUNITY: When reading all channels, do not use a
// slicer at all.
Interval<size_t>
MeasurementAIPS::getChannelRange(const VisSelection &selection) const
{

    // Initialize input channel range.
    Interval<size_t> in(selection.getChannelRange());

    // Initialize output channel range.
    ASSERT(nFreq() > 0);
    Interval<size_t> out(0, nFreq() - 1);

    if(selection.isSet(VisSelection::CHANNEL_START))
    {
        if(in.start > out.end)
        {
            LOG_WARN("Invalid start channel specified; using first channel"
                "available.");
        }
        else
        {
            out.start = in.start;
        }
    }

    if(selection.isSet(VisSelection::CHANNEL_END))
    {
        // NB. We don't have to check that in.end >= out.start: If the selection
        // specifies a start channel, it is guaranteed to be <= any specified
        // end channel. Otherwise, the start channel is 0 and thus automatically
        // <= any specified end channel.
        if(in.end > out.end)
        {
            LOG_WARN("Invalid end channel specified; using last channel"
                " available.");
        }
        else
        {
            out.end = in.end;
        }
    }

    if(itsFreqAxisReversed)
    {
        // Adjust range if channels are in reverse order.
        size_t last = nFreq() - 1;
        return Interval<size_t>(last - out.end, last - out.start);
    }

    return out;
}

Slicer MeasurementAIPS::getCellSlicer(const VisSelection &selection) const
{
    Interval<size_t> range(getChannelRange(selection));
    return Slicer(IPosition(2, 0, range.start),
        IPosition(2, nCorrelations() - 1, range.end),
        Slicer::endIsLast);
}

Slicer MeasurementAIPS::getCovarianceSlicer(const VisSelection &selection,
    const string &column) const
{
    if(column == "WEIGHT")
    {
        return Slicer(IPosition(1, 0), IPosition(1, nCorrelations() - 1),
            Slicer::endIsLast);
    }

    if(column == "WEIGHT_SPECTRUM")
    {
        Interval<size_t> range(getChannelRange(selection));
        return Slicer(IPosition(2, 0, range.start),
            IPosition(2, nCorrelations() - 1, range.end),
            Slicer::endIsLast);
    }

    // Get information about the shape of the column and check the length of
    // each dimension.
    ROTableColumn info(itsMS, column);
    ASSERTSTR(info.shapeColumn().isEqual(IPosition(3, nCorrelations(),
        nCorrelations(), nFreq())), "Covariance column has unexpected shape: "
        << column << " shape: " << info.shapeColumn());

    Interval<size_t> range(getChannelRange(selection));
    return Slicer(IPosition(3, 0, 0, range.start), IPosition(3,
        nCorrelations() - 1, nCorrelations() - 1, range.end),
        Slicer::endIsLast);
}

string MeasurementAIPS::getLinkedCovarianceColumn(const string &column,
    const string &defaultColumn) const
{
    // Fetch the name of the associated covariance column from the specified
    // column's keywords (if available).
    try
    {
        ROTableColumn tableColumn(itsMS, column);
        const TableRecord &keywords = tableColumn.keywordSet();

        if(keywords.isDefined("LOFAR_COVARIANCE_COLUMN"))
        {
            return keywords.asString("LOFAR_COVARIANCE_COLUMN");
        }

        return defaultColumn;
    }
    catch(AipsError &e)
    {
        THROW(BBSKernelException, "Unable to access the keywords of column: "
            << column);
    }
}

void MeasurementAIPS::setLinkedCovarianceColumn(const string &column,
    const string &linkedColumn)
{
    try
    {
        TableColumn tableColumn(itsMS, column);
        TableRecord &keywords = tableColumn.rwKeywordSet();
        keywords.define("LOFAR_COVARIANCE_COLUMN", linkedColumn);
    }
    catch(AipsError &e)
    {
        THROW(BBSKernelException, "Unable to update the keywords of column: "
            << column);
    }
}

Array<Float> MeasurementAIPS::reformatCovarianceArray(const Array<Float> &in,
    unsigned int nCorrelations, unsigned int nFreq, unsigned int nRows) const
{
    switch(in.ndim())
    {
        case 2:
            {
                // Shape should be nCorrelations x nRows.
                ASSERT(in.shape().isEqual(IPosition(2, nCorrelations, nRows)));

                // Allocate space for the result and initialize to zero.
                Array<Float> out(IPosition(4, nCorrelations, nCorrelations,
                    nFreq, nRows), 0.0);

                // Fill the diagonal of the covariance matrix from the input
                // weights. The input values are inverted to convert them to
                // (co)variance.
                IPosition idxIn(2, 0, 0);
                IPosition idxOut(4, 0, 0, 0, 0);
                for(idxOut[3] = 0, idxIn[1] = 0;
                    idxOut[3] < nRows;
                    ++idxOut[3], ++idxIn[1])
                {
                    for(idxOut[2] = 0; idxOut[2] < nFreq; ++idxOut[2])
                    {
                        for(idxOut[1] = 0, idxOut[0] = 0, idxIn[0] = 0;
                            idxOut[1] < nCorrelations;
                            ++idxOut[1], ++idxOut[0], ++idxIn[0])
                        {
                            out(idxOut) = 1.0 / in(idxIn);
                        }
                    }
                }

                return out;
            }

        case 3:
            {
                // Shape should be nCorrelations x nFreq x nRows.
                ASSERT(in.shape().isEqual(IPosition(3, nCorrelations, nFreq,
                     nRows)));

                // Allocate space for the result and initialize to zero.
                Array<Float> out(IPosition(4, nCorrelations, nCorrelations,
                    nFreq, nRows), 0.0);

                // Fill the diagonal of the covariance matrix from the input
                // weights. The input values are inverted to convert them to
                // (co)variance.
                IPosition idxIn(3, 0, 0, 0);
                IPosition idxOut(4, 0, 0, 0, 0);
                for(idxOut[3] = 0, idxIn[2] = 0;
                    idxOut[3] < nRows;
                    ++idxOut[3], ++idxIn[2])
                {
                    for(idxOut[2] = 0, idxIn[1] = 0;
                        idxOut[2] < nFreq;
                        ++idxOut[2], ++idxIn[1])
                    {
                        for(idxOut[1] = 0, idxOut[0] = 0, idxIn[0] = 0;
                            idxOut[1] < nCorrelations;
                            ++idxOut[1], ++idxOut[0], ++idxIn[0])
                        {
                            out(idxOut) = 1.0 / in(idxIn);
                        }
                    }
                }

                return out;
            }

        case 4:
            // The input is already of the right rank. This only occurs if a
            // covariance column was read, so no conversion is needed.
            return in;

        default:
            THROW(BBSKernelException, "Unsupported shape: " << in.shape());
    }
}

VisDimensions MeasurementAIPS::getDimensionsImpl(const Table &tab_selection,
    const Slicer &slicer) const
{
    ASSERTSTR(tab_selection.nrow() > 0, "Cannot determine dimensions of empty"
        " data selection (empty Axis is not supported).");

    VisDimensions dims;

    // Initialize frequency axis.
    double lower, upper;
    if(itsFreqAxisReversed)
    {
        // Correct range if channels are in reverse order.
        const size_t lastChannelIndex = nFreq() - 1;
        lower = grid()[FREQ]->lower(lastChannelIndex - slicer.end()[1]);
        upper = grid()[FREQ]->upper(lastChannelIndex - slicer.start()[1]);
    }
    else
    {
        lower = grid()[FREQ]->lower(slicer.start()[1]);
        upper = grid()[FREQ]->upper(slicer.end()[1]);
    }

    // Get number of channels in the selection.
    const size_t nFreq = slicer.length()[1];

    // Construct frequency axis.
    Axis::ShPtr freqAxis(new RegularAxis(lower, (upper - lower) / nFreq,
        nFreq));

    // Initialize time axis.

    // Find all unique integration cells.
    Table tab_sorted = tab_selection.sort("TIME", Sort::Ascending,
        Sort::HeapSort | Sort::NoDuplicates);

    // Read TIME and INTERVAL column.
    ROScalarColumn<Double> c_time(tab_sorted, "TIME");
    ROScalarColumn<Double> c_interval(tab_sorted, "INTERVAL");
    Vector<Double> time = c_time.getColumn();
    Vector<Double> interval = c_interval.getColumn();

    // Convert to vector<double>.
    vector<double> timeCopy;
    vector<double> intervalCopy;
    time.tovector(timeCopy);
    interval.tovector(intervalCopy);

    // Construct time axis.
    Axis::ShPtr timeAxis(new OrderedAxis(timeCopy, intervalCopy));
    dims.setGrid(Grid(freqAxis, timeAxis));

    // Find all unique baselines.
    Block<String> sortBaselineColumns(2);
    sortBaselineColumns[0] = "ANTENNA1";
    sortBaselineColumns[1] = "ANTENNA2";
    Table tab_baselines = tab_selection.sort(sortBaselineColumns,
        Sort::Ascending, Sort::HeapSort | Sort::NoDuplicates);
    const size_t nBaselines = tab_baselines.nrow();

    // Initialize baseline axis.
    ROScalarColumn<Int> c_antenna1(tab_baselines, "ANTENNA1");
    ROScalarColumn<Int> c_antenna2(tab_baselines, "ANTENNA2");
    Vector<Int> antenna1 = c_antenna1.getColumn();
    Vector<Int> antenna2 = c_antenna2.getColumn();

    BaselineSeq baselines;
    for(size_t i = 0; i < nBaselines; ++i)
    {
        baselines.append(baseline_t(antenna1[i], antenna2[i]));
    }
    dims.setBaselines(baselines);

    // Initialize correlation axis.
    dims.setCorrelations(correlations());

    LOG_DEBUG_STR("Selection contains " << dims.nBaselines() << " baseline(s), "
        << dims.nTime() << " timeslot(s), " << dims.nFreq() << " channel(s),"
        << " and " << dims.nCorrelations() << " correlation(s).");

    return dims;
}

Instrument::Ptr readInstrument(const MeasurementSet &ms,
  unsigned int idObservation)
{
    ROMSObservationColumns observation(ms.observation());
    ASSERT(observation.nrow() > idObservation);
    ASSERT(!observation.flagRow()(idObservation));

    // Get station names and positions in ITRF coordinates.
    ROMSAntennaColumns antenna(ms.antenna());

    // Get station positions.
    MVPosition centroid;
    vector<Station::Ptr> stations(antenna.nrow());
    for(unsigned int i = 0; i < stations.size(); ++i)
    {
        // Get station name and ITRF position.
        MPosition position = MPosition::Convert(antenna.positionMeas()(i),
            MPosition::ITRF)();

        // Store station information.
        stations[i] = readStation(ms, i, antenna.name()(i), position);

        // Update ITRF centroid.
        centroid += position.getValue();
    }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition position;

    // Read observatory name and try to look-up its position.
    const string observatory = observation.telescopeName()(idObservation);
    if(MeasTable::Observatory(position, observatory))
    {
        position = MPosition::Convert(position, MPosition::ITRF)();
    }
    else
    {
        LOG_WARN("Instrument position unknown; will use centroid of stations.");
        ASSERT(antenna.nrow() != 0);
        centroid *= 1.0 / static_cast<double>(antenna.nrow());
        position = MPosition(centroid, MPosition::ITRF);
    }

    return Instrument::Ptr(new Instrument(observatory, position,
      stations.begin(), stations.end()));
}

MDirection readPhaseReference(const MeasurementSet &ms, unsigned int idField)
{
    ROMSFieldColumns field(ms.field());
    ASSERT(field.nrow() > idField);
    ASSERT(!field.flagRow()(idField));

    return field.phaseDirMeas(idField);
}

MDirection readDelayReference(const MeasurementSet &ms, unsigned int idField)
{
    ROMSFieldColumns field(ms.field());
    ASSERT(field.nrow() > idField);
    ASSERT(!field.flagRow()(idField));

    return field.delayDirMeas(idField);
}

MDirection readTileReference(const MeasurementSet &ms, unsigned int idField)
{
    // The MeasurementSet class does not support LOFAR specific columns, so we
    // use ROArrayMeasColumn to read the tile beam reference direction.
    Table tab_field = getSubTable(ms, "FIELD");

    static const String columnName = "LOFAR_TILE_BEAM_DIR";
    if(hasColumn(tab_field, columnName))
    {
        ROArrayMeasColumn<MDirection> c_direction(tab_field, columnName);
        if(c_direction.isDefined(idField))
        {
            return c_direction(idField)(IPosition(1, 0));
        }
    }

    // By default, the tile beam reference direction is assumed to be equal
    // to the station beam reference direction (for backward compatibility,
    // and for non-HBA measurements).
    return readDelayReference(ms, idField);
}

double readFreqReference(const MeasurementSet &ms,
    unsigned int idDataDescription)
{
    // Read spectral window id.
    ROMSDataDescColumns desc(ms.dataDescription());
    ASSERT(desc.nrow() > idDataDescription);
    ASSERT(!desc.flagRow()(idDataDescription));

    const unsigned int idWindow = desc.spectralWindowId()(idDataDescription);

    // Read reference frequency.
    ROMSSpWindowColumns window(ms.spectralWindow());
    ASSERT(window.nrow() > idWindow);
    ASSERT(!window.flagRow()(idWindow));

    return window.refFrequency()(idWindow);
}

namespace
{

bool hasColumn(const Table &table, const string &column)
{
    return table.tableDesc().isColumn(column);
}

bool hasSubTable(const Table &table, const string &name)
{
    return table.keywordSet().isDefined(name);
}

Table getSubTable(const Table &table, const string &name)
{
    return table.keywordSet().asTable(name);
}

Station::Ptr readStation(const Table &table, unsigned int id,
    const string &name, const MPosition &position)
{
    if(!hasSubTable(table, "LOFAR_ANTENNA_FIELD"))
    {
        return Station::Ptr(new Station(name, position));
    }

    Table tab_field = getSubTable(table, "LOFAR_ANTENNA_FIELD");
    tab_field = tab_field(tab_field.col("ANTENNA_ID") == static_cast<Int>(id));

    const size_t nFields = tab_field.nrow();
    if(nFields == 0)
    {
        LOG_WARN_STR("Antenna " << name << " has no associated antenna fields."
          " Beamforming simulation will be switched off for this antenna.");
        return Station::Ptr(new Station(name, position));
    }

    ROScalarColumn<String> c_name(tab_field, "NAME");
    ROArrayQuantColumn<Double> c_position(tab_field, "POSITION", "m");
    ROArrayQuantColumn<Double> c_axes(tab_field, "COORDINATE_AXES", "m");
    ROArrayQuantColumn<Double> c_tile_offset(tab_field, "TILE_ELEMENT_OFFSET",
        "m");
    ROArrayQuantColumn<Double> c_offset(tab_field, "ELEMENT_OFFSET", "m");
    ROArrayColumn<Bool> c_flag(tab_field, "ELEMENT_FLAG");

    Station::Ptr station(new Station(name, position));
    for(size_t i = 0; i < nFields; ++i)
    {
        // Read antenna field center (ITRF).
        Vector<Quantum<Double> > aips_position = c_position(i);
        ASSERT(aips_position.size() == 3);

        Vector3 position = {{aips_position(0).getValue(),
            aips_position(1).getValue(), aips_position(2).getValue()}};

        // Read antenna field coordinate axes (ITRF).
        Matrix<Quantum<Double> > aips_axes = c_axes(i);
        ASSERT(aips_axes.shape().isEqual(IPosition(2, 3, 3)));

        Vector3 P = {{aips_axes(0, 0).getValue(), aips_axes(1, 0).getValue(),
            aips_axes(2, 0).getValue()}};
        Vector3 Q = {{aips_axes(0, 1).getValue(), aips_axes(1, 1).getValue(),
            aips_axes(2, 1).getValue()}};
        Vector3 R = {{aips_axes(0, 2).getValue(), aips_axes(1, 2).getValue(),
            aips_axes(2, 2).getValue()}};

        // Construct AntennaField instance from available information.
        AntennaField::Ptr field(new AntennaField(c_name(i), position, P,
            Q, R));

        // Read offsets (ITRF) of the dipoles within a tile for HBA antenna
        // fields.
        if(c_name(i) != "LBA")
        {
            // Read tile configuration for HBA antenna fields.
            Matrix<Quantum<Double> > aips_offset = c_tile_offset(i);
            ASSERT(aips_offset.nrow() == 3);

            const size_t nElement = aips_offset.ncolumn();
            ASSERTSTR(nElement > 0, "Antenna field #" << i << " of antenna "
                << name << " is reported to be an HBA field, but no HBA tile"
                " layout information is available for it.");
            for(size_t j = 0; j < nElement; ++j)
            {
                Vector3 offset = {{aips_offset(0, j).getValue(),
                    aips_offset(1, j).getValue(),
                    aips_offset(2, j).getValue()}};

                field->appendTileElement(offset);
            }
        }

        // Read element offsets and flags.
        Matrix<Quantum<Double> > aips_offset = c_offset(i);
        Matrix<Bool> aips_flag = c_flag(i);

        const size_t nElement = aips_offset.ncolumn();
        ASSERTSTR(nElement > 0, "Antenna field #" << i << " of antenna " << name
            << " contains no antenna elements.");
        ASSERT(aips_offset.shape().isEqual(IPosition(2, 3, nElement)));
        ASSERT(aips_flag.shape().isEqual(IPosition(2, 2, nElement)));

        for(size_t j = 0; j < nElement; ++j)
        {
            AntennaField::Element element;
            element.offset[0] = aips_offset(0, j).getValue();
            element.offset[1] = aips_offset(1, j).getValue();
            element.offset[2] = aips_offset(2, j).getValue();
            element.flag[0] = aips_flag(0, j);
            element.flag[1] = aips_flag(1, j);

            field->appendElement(element);
        }

        station->append(field);
    }

    return station;
}

} //# namespace unnamed

} //# namespace BBS
} //# namespace LOFAR
