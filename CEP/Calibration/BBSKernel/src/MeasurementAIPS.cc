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

#include <cstring>
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

using namespace casa;

namespace LOFAR
{
namespace BBS
{

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
    initInstrument();

    // Get the phase center of the selected field.
    initPhaseReference();

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
    LOG_DEBUG_STR("Measurement dimensions:\n"
        << ((Measurement*) this)->dimensions());
}

VisDimensions MeasurementAIPS::dimensions(const VisSelection &selection)
    const
{
    Slicer slicer = getCellSlicer(selection);
    Table tab_selection = getVisSelection(itsMainTableView, selection);
    return getDimensionsImpl(tab_selection, slicer);
}

VisBuffer::Ptr MeasurementAIPS::read(const VisSelection &selection,
    const string &column) const
{
    NSTimer readTimer, copyTimer;

    Slicer slicer = getCellSlicer(selection);
    Table tab_selection = getVisSelection(itsMainTableView, selection);

    // Get the dimensions of the visibility data that matches the selection.
    VisDimensions dims(getDimensionsImpl(tab_selection, slicer));

    // Allocate buffer for visibility data.
    VisBuffer::Ptr buffer(new VisBuffer(dims, itsInstrument, itsPhaseReference,
        itsReferenceFreq));

    if(dims.empty())
    {
        return buffer;
    }

    // Initialize all flags to 1, which effectively means all samples are
    // flagged. This way, if certain data is missing in the measurement set
    // the corresponding samples in the buffer will be uninitialized but also
    // flagged.
    buffer->flagsSet(1);

    const size_t nFreq = dims.nFreq();
    const size_t nCorrelations = dims.nCorrelations();

    Table tab_tslot;
    size_t tslot = 0;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < dims.nTime(), "Timeslot out of range.");

        // Get next timeslot.
        tab_tslot = tslotIterator.table();
        size_t nRows = tab_tslot.nrow();

        // Declare all the columns we want to read.
        ROScalarColumn<Int> c_antenna1(tab_tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tab_tslot, "ANTENNA2");
        ROScalarColumn<Bool> c_flag_row(tab_tslot, "FLAG_ROW");
        ROArrayColumn<Bool> c_flag(tab_tslot, "FLAG");
        ROArrayColumn<Complex> c_data(tab_tslot, column);

        // Read data from the MS.
        readTimer.start();
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        Vector<Bool> aips_flag_row = c_flag_row.getColumn();
        Cube<Bool> aips_flag = c_flag.getColumn(slicer);
        Cube<Complex> aips_data = c_data.getColumn(slicer);
        readTimer.stop();

        // Validate shapes.
        ASSERT(aips_antenna1.shape().isEqual(IPosition(1, nRows)));
        ASSERT(aips_antenna2.shape().isEqual(IPosition(1, nRows)));
        ASSERT(aips_flag_row.shape().isEqual(IPosition(1, nRows)));
        IPosition shape = IPosition(3, nCorrelations, nFreq, nRows);
        ASSERT(aips_flag.shape().isEqual(shape));
        ASSERT(aips_data.shape().isEqual(shape));
        ASSERT(aips_flag.contiguousStorage());
        ASSERT(aips_data.contiguousStorage());

        // Use multi_array_ref to ease transposing the data to per baseline
        // timeseries. This also accounts for reversed channels if necessary.
        bool ascending[] = {true, !itsFreqAxisReversed, true};

        boost::multi_array_ref<Bool, 3>::size_type order_flag[] = {2, 1, 0};
        boost::multi_array_ref<Bool, 3>
            flags(const_cast<Bool*>(aips_flag.data()),
                boost::extents[nRows][nFreq][nCorrelations],
                boost::general_storage_order<3>(order_flag, ascending));

        boost::multi_array_ref<Complex, 3>::size_type order_data[] = {2, 1, 0};
        boost::multi_array_ref<Complex, 3>
            samples(const_cast<Complex*>(aips_data.data()),
                boost::extents[nRows][nFreq][nCorrelations],
                boost::general_storage_order<3>(order_data, ascending));

        copyTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            baseline_t baseline(aips_antenna1[i], aips_antenna2[i]);

            // Look up baseline index.
            size_t basel = dims.baselines().index(baseline);
            ASSERT(basel < dims.nBaselines());

            // If the entire row is flagged do nothing (all sample flags have
            // been initialized to 1 above). Otherwise, copy the flags read from
            // disk into the buffer.
            if(!aips_flag_row(i))
            {
                // Copy flags.
                buffer->flags[basel][tslot] = flags[i];
            }

            // Copy visibilities.
            buffer->samples[basel][tslot] = samples[i];
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
    const VisSelection &selection, const string &column, bool writeFlags,
    flag_t flagMask)
{
    NSTimer readTimer, writeTimer;

    // Reopen MS for writing.
    itsMS.reopenRW();
    ASSERT(itsMS.isWritable());

    // Add data column if it does not exist.
    if(!hasColumn(column))
    {
        addDataColumn(column);
    }

    // If the buffer to write is empty, return immediately.
    if(buffer->dimensions().empty())
    {
        return;
    }

    LOG_DEBUG_STR("Writing to column: " << column);

    Slicer slicer = getCellSlicer(selection);
    Table tab_selection = getVisSelection(itsMainTableView, selection);

    // Allocate temporary buffers to be able to reverse frequency channels.
    // TODO: Some performance can be gained by creating a specialized
    // implementation for the case where the channels are in ascending order to
    // avoid the extra copy.
    bool ascending[] = {!itsFreqAxisReversed, true};
    boost::multi_array<Complex, 2>::size_type order_data[] = {1, 0};
    boost::multi_array<Complex, 2>
        sampleBuffer(boost::extents[buffer->nFreq()][buffer->nCorrelations()],
            boost::general_storage_order<2>(order_data, ascending));

    boost::multi_array<Bool, 2>::size_type order_flag[] = {1, 0};
    boost::multi_array<Bool, 2>
        flagBuffer(boost::extents[buffer->nFreq()][buffer->nCorrelations()],
            boost::general_storage_order<2>(order_flag, ascending));

    Table tab_tslot;
    size_t tslot = 0;
    bool mismatch = false;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < buffer->nTime(), "Timeslot out of range.");

        // Extract next timeslot.
        tab_tslot = tslotIterator.table();
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
        }
        writeTimer.stop();

        ++tslot;
        ++tslotIterator;
    }

    // Flush data to disk. This is where most of the time is spent.
    writeTimer.start();
    tab_selection.flush();
    writeTimer.stop();

    if(mismatch)
    {
        LOG_WARN_STR("Time mismatch(es) detected while writing data.");
    }

    LOG_DEBUG_STR("Read time (meta data): " << readTimer);
    LOG_DEBUG_STR("Write time: " << writeTimer);
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

void MeasurementAIPS::initInstrument()
{
    // Get station names and positions in ITRF coordinates.
    ROMSAntennaColumns antenna(itsMS.antenna());
    ROMSObservationColumns observation(itsMS.observation());

    ASSERT(observation.nrow() > itsIdObservation);
    ASSERT(!observation.flagRow()(itsIdObservation));

    // Get instrument name.
    string name(observation.telescopeName()(itsIdObservation));

    // Get station positions.
    vector<Station> stations;
    stations.reserve(antenna.nrow());

    MVPosition centroid;
    for(unsigned int i = 0; i < static_cast<unsigned int>(antenna.nrow()); ++i)
    {
        // Get station name and ITRF position.
        MPosition position = MPosition::Convert(antenna.positionMeas()(i),
            MPosition::ITRF)();

        // Store station information.
        stations.push_back(Station(antenna.name()(i), position));

        // Update ITRF centroid.
        centroid += position.getValue();
    }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition position;
    if(MeasTable::Observatory(position, name))
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

    itsInstrument = Instrument(name, position, stations);
}

void MeasurementAIPS::initPhaseReference()
{
    // Get phase center as RA and DEC (J2000).
    ROMSFieldColumns field(itsMS.field());
    ASSERT(field.nrow() > itsIdField);
    ASSERT(!field.flagRow()(itsIdField));

    itsPhaseReference = MDirection::Convert(field.phaseDirMeas(itsIdField),
        MDirection::J2000)();
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

bool MeasurementAIPS::hasColumn(const string &column) const
{
    return itsMS.tableDesc().isColumn(column);
}

void MeasurementAIPS::addDataColumn(const string &column)
{
    LOG_INFO_STR("Adding column \"" << column << "\".");

    // Added column should get the same shape as the other data columns in
    // the MS.
    ArrayColumnDesc<Complex> columnDescriptor(column, IPosition(2,
        nCorrelations(), nFreq()), ColumnDesc::FixedShape);

    // Create storage manager. Tile size specification taken from the
    // MSCreate class in the CEP/MS package.
    TiledColumnStMan storageManager("Tiled_" + column, IPosition(3,
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


// we can not use MeasurementAIPS::addDataColumn() anymore, because we also
// have to write the specific column description with keyword
void MeasurementAIPS::addClearcalColumns(void)
{
    itsMS.reopenRW();
    ASSERT(itsMS.isWritable());
   
    // prepare to column keyword: "CHANNEL_SELECTION" (required by casapy)
    Matrix<Int> selection(2, 1);
    selection(0, 0) = 0;         //start
    selection(1, 0) = nFreq();   //number of channels in the MS:

    if(!this->hasColumn("MODEL_DATA"))         // if no MODEL_DATA column, add it
    {
       LOG_INFO_STR("MeasurementAIPS::addClearcalColumns() MODEL_DATA");
       addDataColumn("MODEL_DATA");
      
       // Add column keyword: "CHANNEL_SELECTION"
       ArrayColumn<Complex> modelData(itsMS, "MODEL_DATA");
       modelData.rwKeywordSet().define("CHANNEL_SELECTION", selection);
      
       // casapy also initializes the MODEL_DATA column with (1,0)
       Matrix<Complex> defaultModelData(4, 1, Complex(1,0));
       modelData.fillColumn(defaultModelData);
    }
    if(!this->hasColumn("CORRECTED_DATA"))     // if no CORRECTED_DATA column, add it
    {
       LOG_INFO_STR("MeasurementAIPS::addClearcalColumns() CORRECTED_DATA");
       addDataColumn("CORRECTED_DATA");
    }
   
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

    const pair<double, double> &timeRange = selection.getTimeRange();
    if(selection.isSet(VisSelection::TIME_START))
    {
        filter = filter && table.col("TIME") >= timeRange.first;
    }

    if(selection.isSet(VisSelection::TIME_END))
    {
        filter = filter && table.col("TIME") <= timeRange.second;
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

// NOTE: OPTIMIZATION OPPORTUNITY: when reading all channels, do not use a
// slicer at all.
Slicer MeasurementAIPS::getCellSlicer(const VisSelection &selection) const
{
    // Validate and set selection.
    pair<size_t, size_t> range = selection.getChannelRange();

    size_t startChannel = 0;
    size_t endChannel = nFreq() - 1;

    if(selection.isSet(VisSelection::CHANNEL_START))
    {
        startChannel = range.first;
    }

    if(selection.isSet(VisSelection::CHANNEL_END))
    {
        if(range.second > endChannel)
        {
            LOG_WARN("Invalid end channel specified; using last channel"
                " instead.");
        }
        else
        {
            endChannel = range.second;
        }
    }

    IPosition start(2, 0, startChannel);
    IPosition end(2, nCorrelations() - 1, endChannel);

    if(itsFreqAxisReversed)
    {
        // Adjust range if channels are in reverse order.
        size_t lastChannelIndex = nFreq() - 1;
        start = IPosition(2, 0, lastChannelIndex - endChannel);
        end = IPosition(2, nCorrelations() - 1, lastChannelIndex
            - startChannel);
    }

    return Slicer(start, end, Slicer::endIsLast);
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

} //# namespace BBS
} //# namespace LOFAR
