//# MeasurementAIPS.cc: Specialisation of Measurement that understands the
//# AIPS++ measurement set format.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
    initPhaseCenter();

    // Select a single measurement from the measurement set.
    // TODO: At the moment we use (OBSERVATION_ID, FIELD_ID, DATA_DESC_ID) as
    // the key to select a single measurement. This may not be strict enough,
    // i.e. different measurements may still have the same key.
    itsMainTableView =
        itsMS(itsMS.col("OBSERVATION_ID") == static_cast<Int>(idObservation)
        && itsMS.col("FIELD_ID") == static_cast<Int>(idField)
        && itsMS.col("DATA_DESC_ID") == static_cast<Int>(idDataDescription));

    // Get the dimensions (time, frequency, baselines, polarization products)
    // of the measurement.
    initDimensions();

    LOG_DEBUG_STR("Measurement " << idObservation << " contains "
        << itsMainTableView.nrow() << " row(s).");
    LOG_DEBUG_STR("Measurement dimensions: " << endl << itsDimensions);
}


VisDimensions MeasurementAIPS::getDimensions(const VisSelection &selection)
    const
{
    Slicer slicer = getCellSlicer(selection);
    Table tab_selection = getTableSelection(itsMainTableView, selection);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection did not match any"
        " rows.");

    return getDimensionsImpl(tab_selection, slicer);
}


VisData::Ptr MeasurementAIPS::read(const VisSelection &selection,
    const string &column, bool readUVW) const
{
    NSTimer readTimer, copyTimer;

    Slicer slicer = getCellSlicer(selection);
    Table tab_selection = getTableSelection(itsMainTableView, selection);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection did not match any"
        " rows.");

    VisDimensions visDims(getDimensionsImpl(tab_selection, slicer));
    VisData::Ptr buffer(new VisData(visDims));

    const VisDimensions &dims = buffer->getDimensions();
    const size_t nChannels = dims.getChannelCount();
    const size_t nPolarizations = dims.getPolarizationCount();

    Table tab_tslot;
    size_t tslot = 0;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < dims.getTimeslotCount(), "Timeslot out of range.");

        // Get next timeslot.
        tab_tslot = tslotIterator.table();
        size_t nRows = tab_tslot.nrow();
        ASSERTSTR(nRows <= dims.getBaselineCount(), "Measurement contains"
            " multiple samples with the same timestamp for a single baseline.");

        // Declare all the columns we want to read.
        ROScalarColumn<Int> c_antenna1(tab_tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tab_tslot, "ANTENNA2");
        ROScalarColumn<Bool> c_flag_row(tab_tslot, "FLAG_ROW");
        ROArrayColumn<Double> c_uvw(tab_tslot, "UVW");
        ROArrayColumn<Complex> c_data(tab_tslot, column);
        ROArrayColumn<Bool> c_flag(tab_tslot, "FLAG");

        // Read data from the MS.
        readTimer.start();
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        Vector<Bool> aips_flag_row = c_flag_row.getColumn();
        Matrix<Double> aips_uvw;
        if(readUVW)
        {
             aips_uvw = c_uvw.getColumn();
        }
        Cube<Complex> aips_data = c_data.getColumn(slicer);
        Cube<Bool> aips_flag = c_flag.getColumn(slicer);
        readTimer.stop();

        // Validate shapes.
        ASSERT(aips_antenna1.shape().isEqual(IPosition(1, nRows)));
        ASSERT(aips_antenna2.shape().isEqual(IPosition(1, nRows)));
        ASSERT(aips_flag_row.shape().isEqual(IPosition(1, nRows)));
        ASSERT(!readUVW || aips_uvw.shape().isEqual(IPosition(2, 3, nRows)));
        IPosition shape = IPosition(3, nPolarizations, nChannels, nRows);
        ASSERT(aips_data.shape().isEqual(shape));
        ASSERT(aips_flag.shape().isEqual(shape));
        ASSERT(aips_uvw.contiguousStorage());
        ASSERT(aips_data.contiguousStorage());
        ASSERT(aips_flag.contiguousStorage());

        // The const_cast<>() call below is needed because multi_array_ref
        // expects a non-const pointer. This does not break the const semantics
        // as we never write to the array.
        Double *ptr_uvw = const_cast<Double*>(aips_uvw.data());
        boost::multi_array_ref<double, 2> uvw(ptr_uvw,
                boost::extents[nRows][3]);

        // Use multi_array_ref to ease transposing the data to per baseline
        // timeseries. This also accounts for reversed channels if necessary.
        bool ascending[] = {true, !itsFreqAxisReversed, true};
        boost::multi_array_ref<sample_t, 3>::size_type order_data[] = {2, 1, 0};
        Complex *ptr_data = const_cast<Complex*>(aips_data.data());
        boost::multi_array_ref<sample_t, 3>
            data(reinterpret_cast<sample_t*>(ptr_data),
                boost::extents[nRows][nChannels][nPolarizations],
                boost::general_storage_order<3>(order_data, ascending));

        boost::multi_array_ref<flag_t, 3>::size_type order_flag[] = {2, 1, 0};
//        Bool *ptr_flag = const_cast<Bool*>(aips_flag.data());
        Bool *ptr_flag = const_cast<Bool*>(aips_flag.data());
//        boost::multi_array_ref<flag_t, 3> flag(ptr_flag,
//                boost::extents[nRows][nChannels][nPolarizations],
//                boost::general_storage_order<3>(order_flag, ascending));
        boost::multi_array_ref<flag_t, 3>
            flag(reinterpret_cast<flag_t*>(ptr_flag),
                boost::extents[nRows][nChannels][nPolarizations],
                boost::general_storage_order<3>(order_flag, ascending));

        copyTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            size_t basel = dims.getBaselineIndex(baseline_t(aips_antenna1[i],
                aips_antenna2[i]));

            // Flag timeslot as available.
            buffer->tslot_flag[basel][tslot] = 0;

            // Copy row (timeslot) flag.
            if(aips_flag_row(i))
            {
                buffer->tslot_flag[basel][tslot] |=
                    VisData::FLAGGED_IN_INPUT;
            }

            // Copy UVW.
            if(readUVW)
            {
                buffer->uvw[basel][tslot] = uvw[i];
            }

            // Copy visibilities.
            buffer->vis_data[basel][tslot] = data[i];
            // Copy flags.
            buffer->vis_flag[basel][tslot] = flag[i];
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



void MeasurementAIPS::write(const VisSelection &selection,
    VisData::Ptr buffer, const string &column, bool writeFlags)
{
    NSTimer readTimer, writeTimer;

    // Reopen MS for writing.
    itsMS.reopenRW();
    ASSERT(itsMS.isWritable());

    // Add column if it does not exist.
    if(!itsMS.tableDesc().isColumn(column))
    {
        LOG_INFO_STR("Adding column \"" << column << "\".");

        // Added column should get the same shape as the other data columns in
        // the MS.
        ArrayColumnDesc<Complex> columnDescriptor(column,
            IPosition(2, itsDimensions.getPolarizationCount(),
                itsDimensions.getChannelCount()),
            ColumnDesc::FixedShape);

        // Create storage manager. Tile size specification taken from the
        // MSCreate class in the CEP/MS package.
        TiledColumnStMan storageManager("Tiled_" + column, IPosition(3,
            itsDimensions.getPolarizationCount(),
            itsDimensions.getChannelCount(),
            std::max(size_t(1), 4096 / itsDimensions.getChannelCount())));

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
    LOG_DEBUG_STR("Writing to column: " << column);

    Slicer slicer = getCellSlicer(selection);
    Table tab_selection = getTableSelection(itsMainTableView, selection);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection did not match any"
        " rows.");

    const VisDimensions &dims = buffer->getDimensions();
    const size_t nChannels = dims.getChannelCount();
    const size_t nPolarizations = dims.getPolarizationCount();
    const Grid &grid = dims.getGrid();

    // Allocate temporary buffers to be able to reverse frequency channels.
    // TODO: Some performance can be gained by creating a specialized
    // implementation for the case where the channels are in ascending order to
    // avoid the extra copy.
    bool ascending[] = {!itsFreqAxisReversed, true};
    boost::multi_array<sample_t, 2>::size_type order_data[] = {1, 0};
    boost::multi_array<sample_t, 2>
        visBuffer(boost::extents[nChannels][nPolarizations],
            boost::general_storage_order<2>(order_data, ascending));

    boost::multi_array<flag_t, 2>::size_type order_flag[] = {1, 0};
    boost::multi_array<flag_t, 2>
        flagBuffer(boost::extents[nChannels][nPolarizations],
            boost::general_storage_order<2>(order_flag, ascending));

    Table tab_tslot;
    size_t tslot = 0;
    bool mismatch = false;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < dims.getTimeslotCount(), "Timeslot out of range.");

        // Extract next timeslot.
        tab_tslot = tslotIterator.table();
        size_t nRows = tab_tslot.nrow();
        ASSERTSTR(nRows <= dims.getBaselineCount(), "Measurement contains"
            " multiple samples with the same timestamp for a single baseline.");

        // TODO: Should use TIME_CENTROID here (centroid of exposure)? NB. The
        // TIME_CENTROID may be different for each baseline!
        ROScalarColumn<Double> c_time(tab_tslot, "TIME");
        ROScalarColumn<Int> c_antenna1(tab_tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tab_tslot, "ANTENNA2");
        ScalarColumn<Bool> c_flag_row(tab_tslot, "FLAG_ROW");
        ArrayColumn<Complex> c_data(tab_tslot, column);
        ArrayColumn<Bool> c_flag(tab_tslot, "FLAG");

        // Read meta data.
        readTimer.start();
        Vector<Double> aips_time = c_time.getColumn();
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        readTimer.stop();

        mismatch = mismatch && (grid[TIME]->center(tslot) == aips_time(0));

        writeTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            size_t basel = dims.getBaselineIndex(baseline_t(aips_antenna1[i],
                    aips_antenna2[i]));

            if(writeFlags)
            {
                // Write row flag.
                c_flag_row.put(i, buffer->tslot_flag[basel][tslot]);

                // Copy flags and optionally reverse.
                flagBuffer = buffer->vis_flag[basel][tslot];

                // Write visibility flags.
                Array<Bool> vis_flag(IPosition(2, nPolarizations, nChannels),
                    reinterpret_cast<Bool*>(flagBuffer.data()), SHARE);
                c_flag.putSlice(i, slicer, vis_flag);
            }

            // Copy visibilities and optionally reverse.
			visBuffer = buffer->vis_data[basel][tslot];

			// Write visibilities.
            Array<Complex> vis_data(IPosition(2, nPolarizations, nChannels),
                reinterpret_cast<Complex*>(visBuffer.data()), SHARE);
            c_data.putSlice(i, slicer, vis_data);
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
        LOG_WARN_STR("Time mismatches detected while writing data.");
    }

    LOG_DEBUG_STR("Read time (meta data): " << readTimer);
    LOG_DEBUG_STR("Write time: " << writeTimer);
}


void MeasurementAIPS::initInstrument()
{
    // Get station names and positions in ITRF coordinates.
    ROMSAntennaColumns antenna(itsMS.antenna());
    ROMSObservationColumns observation(itsMS.observation());

    ASSERT(observation.nrow() > itsIdObservation);
    ASSERT(!observation.flagRow()(itsIdObservation));

    itsInstrument.name = observation.telescopeName()(itsIdObservation);
    itsInstrument.stations.reserve(antenna.nrow());

    MVPosition centroid;
    for(unsigned int i = 0; i < static_cast<unsigned int>(antenna.nrow()); ++i)
    {
        // Get station name and ITRF position.
        Station station;
        station.name = antenna.name()(i);
        station.position = MPosition::Convert(antenna.positionMeas()(i),
            MPosition::ITRF)();

        // Store station information.
        itsInstrument.stations.push_back(station);

        // Update ITRF centroid.
        centroid += station.position.getValue();
    }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition position;
    if(MeasTable::Observatory(position, itsInstrument.name))
    {
        itsInstrument.position = MPosition::Convert(position,
            MPosition::ITRF)();
    }
    else
    {
        LOG_WARN("Instrument position unknown; will use centroid of stations.");
        ASSERT(antenna.nrow() != 0);
        centroid *= 1.0 / static_cast<double>(antenna.nrow());
        itsInstrument.position = MPosition(centroid, MPosition::ITRF);
    }
}


void MeasurementAIPS::initPhaseCenter()
{
    // Get phase center as RA and DEC (J2000).
    ROMSFieldColumns field(itsMS.field());
    ASSERT(field.nrow() > itsIdField);
    ASSERT(!field.flagRow()(itsIdField));

    itsPhaseCenter = MDirection::Convert(field.phaseDirMeas(itsIdField),
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

    const unsigned int nChannels = window.numChan()(idWindow);
    Vector<Double> frequency = window.chanFreq()(idWindow);
    Vector<Double> width = window.chanWidth()(idWindow);

    ASSERT(frequency.nelements() == nChannels);
    ASSERT(width.nelements() == nChannels);
    // TODO: Technically, checking for equal channel widths is not enough,
    // because there could still be gaps between channels even though the
    // widths are all equal (this is not prevented by the MS 2.0 standard).
    ASSERTSTR(allEQ(width, width(0)),
        "Channels width is not the same for all channels. This is not supported"
        " yet.");

    double lower = frequency(0) - 0.5 * width(0);
    double upper = frequency(nChannels - 1) + 0.5 * width(nChannels - 1);
    if(frequency(0) > frequency(nChannels - 1))
    {
        LOG_INFO_STR("Channels are in reverse order.");

        // Correct upper and lower boundary.
        lower = frequency(nChannels - 1) - 0.5 * width(nChannels - 1);
        upper = frequency(0) + 0.5 * width(0);
        itsFreqAxisReversed = true;
    }

    // Construct frequency axis.
    Axis::ShPtr freqAxis(new RegularAxis(lower, (upper - lower) / nChannels,
        nChannels));

    // Extract time axis based on TIME column (mid-point of integration
    // interval).
    // TODO: Should use TIME_CENTROID here (centroid of exposure)? NB. UVW is
    // given of the TIME_CENTROID, not for TIME! NB. The TIME_CENTROID may be
    // different for each baseline!

    // Find all unique integration cells.
    Table tab_sorted = itsMainTableView.sort("TIME", Sort::Ascending,
        Sort::QuickSort + Sort::NoDuplicates);

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
    itsDimensions.setGrid(Grid(freqAxis, timeAxis));

    // Find all unique baselines.
    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "ANTENNA2";
    Table tab_baselines = itsMainTableView.sort(sortColumns, Sort::Ascending,
        Sort::QuickSort + Sort::NoDuplicates);
    const unsigned int nBaselines = tab_baselines.nrow();

    // Initialize baseline axis.
    ROScalarColumn<Int> c_antenna1(tab_baselines, "ANTENNA1");
    ROScalarColumn<Int> c_antenna2(tab_baselines, "ANTENNA2");
    Vector<Int> antenna1 = c_antenna1.getColumn();
    Vector<Int> antenna2 = c_antenna2.getColumn();

    vector<baseline_t> baselines;
    baselines.reserve(nBaselines);
    for(unsigned int i = 0; i < nBaselines; ++i)
    {
        baselines.push_back(baseline_t(antenna1[i], antenna2[i]));
    }
    itsDimensions.setBaselines(baselines);

    // Initialize polarization product axis.
    ROMSPolarizationColumns polarization(itsMS.polarization());
    ASSERT(polarization.nrow() > idPolarization);
    ASSERT(!polarization.flagRow()(idPolarization));

    Vector<Int> products = polarization.corrType()(idPolarization);

    vector<string> names;
    names.reserve(products.nelements());
    for(unsigned int i = 0; i < products.nelements(); ++i)
    {
        names.push_back(Stokes::name(Stokes::type(products(i))));
    }
    itsDimensions.setPolarizations(names);
}


// NOTE: OPTIMIZATION OPPORTUNITY: Cache implementation specific selection
// within a specialization of VisSelection or VisData.
Table MeasurementAIPS::getTableSelection(const Table &table,
    const VisSelection &selection) const
{
    // If no selection criteria are specified, return immediately.
    if(selection.empty())
    {
        return table;
    }

    TableExprNode filter(true);

    const pair<double, double> &timeRange = selection.getTimeRange();
    if(selection.isSet(VisSelection::TIME_START))
    {
        filter = filter && table.col("TIME") >= timeRange.first;
    }

    if(selection.isSet(VisSelection::TIME_END))
    {
        filter = filter && table.col("TIME") <= timeRange.second;
    }

    if(selection.isSet(VisSelection::STATIONS))
    {
        const set<string> &regexSet = selection.getStations();
        set<size_t> stationSelection;

        try
        {
            for(set<string>::const_iterator it = regexSet.begin();
                it != regexSet.end();
                ++it)
            {
                Regex regex(Regex::fromPattern(*it));

                // If the name of a station matches the pattern, add it to the
                // selection.
                for(size_t i = 0; i < itsInstrument.stations.size(); ++i)
                {
                    String name(itsInstrument.stations[i].name);

                    if(name.matches(regex))
                    {
                        stationSelection.insert(i);
                    }
                }
            }
        }
        catch(std::exception &ex)
        {
            THROW(BBSKernelException, "Encountered an invalid station"
               " selection criterium.");
        }

        if(stationSelection.empty())
        {
            LOG_WARN_STR("Station selection criteria did not maych any"
                " station in the MS; criteria will be ignored.");
        }
        else
        {
            TableExprNodeSet selectionExpr;
            for(set<size_t>::const_iterator it = stationSelection.begin();
                it != stationSelection.end();
                ++it)
            {
                selectionExpr.add(TableExprNodeSetElem(static_cast<Int>(*it)));
            }

            filter = filter && table.col("ANTENNA1").in(selectionExpr)
                && table.col("ANTENNA2").in(selectionExpr);
        }
    }

    if(selection.isSet(VisSelection::BASELINE_FILTER))
    {
        if(selection.getBaselineFilter() == VisSelection::AUTO)
        {
            filter = filter && (table.col("ANTENNA1") == table.col("ANTENNA2"));
        }
        else if(selection.getBaselineFilter() == VisSelection::CROSS)
        {
            filter = filter && (table.col("ANTENNA1") != table.col("ANTENNA2"));
        }
    }

    if(selection.isSet(VisSelection::POLARIZATIONS))
    {
        LOG_WARN_STR("Polarization selection not yet implemented; all available"
            " polarizations will be used.");
    }

    return table(filter);
}


// NOTE: OPTIMIZATION OPPORTUNITY: when reading all channels, do not use a
// slicer at all.
Slicer MeasurementAIPS::getCellSlicer(const VisSelection &selection) const
{
    // Validate and set selection.
    pair<size_t, size_t> range = selection.getChannelRange();

    size_t startChannel = 0;
    size_t endChannel = itsDimensions.getChannelCount() - 1;

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
	IPosition end(2, itsDimensions.getPolarizationCount() - 1, endChannel);

    if(itsFreqAxisReversed)
    {
        // Adjust range if channels are in reverse order.
        size_t lastChannelIndex = itsDimensions.getChannelCount() - 1;
        start = IPosition(2, 0, lastChannelIndex - endChannel);
        end = IPosition(2, itsDimensions.getPolarizationCount() - 1,
            lastChannelIndex - startChannel);
    }

    return Slicer(start, end, Slicer::endIsLast);
}


VisDimensions MeasurementAIPS::getDimensionsImpl(const Table tab_selection,
    const Slicer slicer) const
{
    VisDimensions dims;

    const Grid &grid = itsDimensions.getGrid();

    // Initialize frequency axis.
    double lower, upper;
    if(itsFreqAxisReversed)
    {
        // Correct range if channels are in reverse order.
        const size_t lastChannelIndex = itsDimensions.getChannelCount() - 1;
        lower = grid[FREQ]->lower(lastChannelIndex - slicer.end()[1]);
        upper = grid[FREQ]->upper(lastChannelIndex - slicer.start()[1]);
    }
    else
    {
        lower = grid[FREQ]->lower(slicer.start()[1]);
        upper = grid[FREQ]->upper(slicer.end()[1]);
    }

    // Get number of channels in the selection.
    const unsigned int nChannels = slicer.length()[1];

    // Construct frequency axis.
    Axis::ShPtr freqAxis(new RegularAxis(lower, (upper - lower) / nChannels,
        nChannels));

    // Initialize time axis.

    // Find all unique integration cells.
    Table tab_sorted = tab_selection.sort("TIME", Sort::Ascending,
        Sort::QuickSort + Sort::NoDuplicates);

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
        Sort::Ascending, Sort::QuickSort + Sort::NoDuplicates);
    const unsigned int nBaselines = tab_baselines.nrow();

    LOG_DEBUG_STR("Selection contains " << nBaselines << " baseline(s), "
        << timeAxis->size() << " timeslot(s), " << freqAxis->size()
        << " channel(s), and " << itsDimensions.getPolarizationCount()
        << " polarization(s).");

    // Initialize baseline axis.
    ROScalarColumn<Int> c_antenna1(tab_baselines, "ANTENNA1");
    ROScalarColumn<Int> c_antenna2(tab_baselines, "ANTENNA2");
    Vector<Int> antenna1 = c_antenna1.getColumn();
    Vector<Int> antenna2 = c_antenna2.getColumn();

    vector<baseline_t> baselines;
    baselines.reserve(nBaselines);
    for(unsigned int i = 0; i < nBaselines; ++i)
    {
        baselines.push_back(baseline_t(antenna1[i], antenna2[i]));
    }
    dims.setBaselines(baselines);

    // Initialize polarization axis.
    dims.setPolarizations(itsDimensions.getPolarizations());

    return dims;
}

} //# namespace BBS
} //# namespace LOFAR
