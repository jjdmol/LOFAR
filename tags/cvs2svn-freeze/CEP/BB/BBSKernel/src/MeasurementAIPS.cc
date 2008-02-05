//# MeasurementAIPS.cc: 
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

#include <cstring>
#include <Common/Timer.h>
#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

//#include <casa/Arrays/Slicer.h>
//#include <casa/Arrays/Vector.h>
// Vector2.cc: necessary to instantiate .tovector()
//#include <casa/Arrays/Vector2.cc>

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

//#include <tables/Tables/TableDesc.h>
//#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/TiledColumnStMan.h>

//#include <tables/Tables/ArrColDesc.h>
//#include <tables/Tables/TiledColumnStMan.h>
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

#include <casa/Utilities/GenSort.h>

#include <Common/lofar_iomanip.h>

using namespace casa;
using namespace std;

namespace LOFAR
{
namespace BBS 
{

MeasurementAIPS::MeasurementAIPS(string filename,
        uint observationId,
        uint dataDescriptionId,
        uint fieldId)
{
    itsMS = MeasurementSet(filename);

    ROMSAntennaColumns antenna(itsMS.antenna());
    ROMSDataDescColumns description(itsMS.dataDescription());
    ROMSFieldColumns field(itsMS.field());
    ROMSObservationColumns observation(itsMS.observation());
    ROMSPolarizationColumns polarization(itsMS.polarization());
    ROMSSpWindowColumns window(itsMS.spectralWindow());

    initObservationInfo(observation, observationId);
    initInstrumentInfo(antenna, observation, observationId);
    initFieldInfo(field, fieldId);

    // Read polarization id and spectral window id.
    ASSERT(description.nrow() > dataDescriptionId);
    ASSERT(!description.flagRow()(dataDescriptionId));
    Int polarizationId = description.polarizationId()(dataDescriptionId);
    Int windowId = description.spectralWindowId()(dataDescriptionId);

    initPolarizationInfo(polarization, polarizationId);
    initSpectralInfo(window, windowId);

    // Select all rows that match the specified observation.
    itsMS = itsMS(itsMS.col("OBSERVATION_ID") == static_cast<Int>(observationId)
        && itsMS.col("DATA_DESC_ID") == static_cast<Int>(dataDescriptionId)
        && itsMS.col("FIELD_ID") == static_cast<Int>(fieldId));

    // Sort MS on TIME.
    itsMS.sort("TIME");

    LOG_DEBUG_STR("Measurement " << observationId << " contains "
        << itsMS.nrow() << " row(s).");
}


MeasurementAIPS::~MeasurementAIPS()
{
    itsMS = MeasurementSet();
}


VisGrid MeasurementAIPS::grid(const VisSelection &selection) const
{
    TableExprNode taqlExpr = getTAQLExpression(selection);
    Slicer slicer = getCellSlicer(selection);

    Table tab_selection = itsMS(taqlExpr);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection empty!");

    return grid(tab_selection, slicer);
}


VisData::Pointer MeasurementAIPS::read(const VisSelection &selection,
    const string &column, bool readUVW) const
{
    NSTimer readTimer, copyTimer;

    Slicer slicer = getCellSlicer(selection);
    TableExprNode taqlExpr = getTAQLExpression(selection);
    Table tab_selection = itsMS(taqlExpr);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection empty!");

    VisGrid visGrid(grid(tab_selection, slicer));
    VisData::Pointer buffer(new VisData(visGrid));

    size_t nChannels = buffer->grid.freq.size();
    size_t nPolarizations = buffer->grid.polarizations.size();

    Table tab_tslot;
    size_t tslot = 0;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERT(tslot < buffer->grid.time.size());

        // Get next timeslot.
        tab_tslot = tslotIterator.table();
        size_t nRows = tab_tslot.nrow();

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
        ASSERT(!readUVW
            || aips_uvw.shape().isEqual(IPosition(2, 3, nRows)));
        IPosition shape = IPosition(3, nPolarizations, nChannels, nRows);
        ASSERT(aips_data.shape().isEqual(shape));
        ASSERT(aips_flag.shape().isEqual(shape));
        ASSERT(aips_uvw.contiguousStorage());
        ASSERT(aips_data.contiguousStorage());
        ASSERT(aips_flag.contiguousStorage());

        // The const_cast<>() call below is needed becase multi_array_ref
        // expects a non-const pointer. This does not break the const semantics
        // as we never write to the array.
        Double *ptr_uvw = const_cast<Double*>(aips_uvw.data());
        boost::multi_array_ref<double, 2> uvw(ptr_uvw,
                boost::extents[nRows][3]);

        Complex *ptr_data = const_cast<Complex*>(aips_data.data());
        boost::multi_array_ref<sample_t, 3>
            data(reinterpret_cast<sample_t*>(ptr_data),
                boost::extents[nRows][nChannels][nPolarizations]);

        Bool *ptr_flag = const_cast<Bool*>(aips_flag.data());
        boost::multi_array_ref<flag_t, 3> flag(ptr_flag,
                boost::extents[nRows][nChannels][nPolarizations]);

        copyTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            size_t basel = 
                buffer->getBaselineIndex(baseline_t(aips_antenna1[i], aips_antenna2[i]));

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
    VisData::Pointer buffer, const string &column, bool writeFlags)
{
    NSTimer readTimer, writeTimer;

    // Reopen MS for writing.
    itsMS.reopenRW();
    ASSERT(itsMS.isWritable());

    ASSERTSTR(itsMS.tableDesc().isColumn(column), "Attempt to write to non-"
        "existent column '" << column << "'.");

/*
    // Add column if it does not exist.
    // TODO: Check why the AIPS++ imager does not recognize these columns.
    if(!itsMS.tableDesc().isColumn(column))
    {
        LOG_INFO_STR("Adding column '" << column << "'.");

        // Added column should get the same shape as the other data columns in
        // the MS.
        ArrayColumnDesc<Complex> descriptor(column,
            IPosition(2, getPolarizationCount(), getChannelCount()),
            ColumnDesc::FixedShape);
        TiledColumnStMan storageManager("Tiled_" + column,
            IPosition(3, getPolarizationCount(), getChannelCount(), 1));

        itsMS.addColumn(descriptor, storageManager);
        itsMS.flush();
    }
*/
    LOG_DEBUG_STR("Writing to column: " << column);

    Slicer slicer = getCellSlicer(selection);
    TableExprNode taqlExpr = getTAQLExpression(selection);
    Table tab_selection = itsMS(taqlExpr);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection empty!");

    size_t nChannels = buffer->grid.freq.size();
    size_t nPolarizations = buffer->grid.polarizations.size();

    Table tab_tslot;
    size_t tslot = 0;
    bool mismatch = false;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslot < buffer->grid.time.size(),
            "Timeslot out of range!");

        // Extract next timeslot.
        tab_tslot = tslotIterator.table();
        size_t nRows = tab_tslot.nrow();

        // TODO: Should use TIME_CENTROID here (centroid of exposure)?
        // NOTE: TIME_CENTROID may be different for each baseline!
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

        mismatch = mismatch && (buffer->grid.time(tslot) == aips_time(0));

        writeTimer.start();
        for(uInt i = 0; i < nRows; ++i)
        {
            // Get time sequence for this baseline.
            size_t basel =
                buffer->getBaselineIndex(baseline_t(aips_antenna1[i], aips_antenna2[i]));

            if(writeFlags)
            {
                // Write row flag.
                c_flag_row.put(i, buffer->tslot_flag[basel][tslot]);

                // Write visibility flags.
                Array<Bool> vis_flag(IPosition(2, nPolarizations, nChannels),
                    &(buffer->vis_flag[basel][tslot][0][0]), SHARE);
                c_flag.putSlice(i, slicer, vis_flag);
            }

            // Write visibilities.
            Array<Complex> vis_data(IPosition(2, nPolarizations, nChannels),
                reinterpret_cast<Complex*>
                    (&(buffer->vis_data[basel][tslot][0][0])), SHARE);
            c_data.putSlice(i, slicer, vis_data);
        }
        writeTimer.stop();

        ++tslot;
        ++tslotIterator;
    }
    tab_selection.flush();

    if(mismatch)
    {
        LOG_WARN_STR("Time mismatches detected while writing data.");
    }

    LOG_DEBUG_STR("Read time (meta data): " << readTimer);
    LOG_DEBUG_STR("Write time: " << writeTimer);
}


void MeasurementAIPS::initObservationInfo
    (const ROMSObservationColumns &observation, uint id)
{
    ASSERT(observation.nrow() > id);
    ASSERT(!observation.flagRow()(id));

    const Vector<MEpoch> &range = observation.timeRangeMeas()(id);
    itsTimeRange = make_pair(range(0), range(1));
}


void MeasurementAIPS::initInstrumentInfo(const ROMSAntennaColumns &antenna,
    const ROMSObservationColumns &observation, uint id)
{
    // Get station names and positions in ITRF coordinates.
    ASSERT(observation.nrow() > id);
    ASSERT(!observation.flagRow()(id));

    itsInstrument.name = observation.telescopeName()(id);
    itsInstrument.stations.resize(antenna.nrow());

    MVPosition stationCentroid;
    for(size_t i = 0; i < static_cast<size_t>(antenna.nrow()); ++i)
    {
        // Store station name and update index.
        itsInstrument.stations[i].name = antenna.name()(i);

        // Store station position.
        MPosition position = antenna.positionMeas()(i);
        itsInstrument.stations[i].position =
            MPosition::Convert(position, MPosition::ITRF)();

        // Update centroid.
        stationCentroid += itsInstrument.stations[i].position.getValue();
    }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition instrumentPosition;
    if(MeasTable::Observatory(instrumentPosition, itsInstrument.name))
    {
        itsInstrument.position =
            MPosition::Convert(instrumentPosition, MPosition::ITRF)();
    }
    else
    {
        LOG_WARN("Instrument position unknown; will use centroid of stations.");
        stationCentroid *= (1.0
            / static_cast<double>(itsInstrument.stations.size()));
        itsInstrument.position = MPosition(stationCentroid, MPosition::ITRF);
    }
}


void MeasurementAIPS::initSpectralInfo(const ROMSSpWindowColumns &window,
    uint id)
{
    ASSERT(window.nrow() > id);
    ASSERT(!window.flagRow()(id));

    size_t nChannels = window.numChan()(id);

    Vector<Double> frequency = window.chanFreq()(id);
    Vector<Double> width = window.chanWidth()(id);

    ASSERT(frequency.nelements() == nChannels);
    ASSERT(width.nelements() == nChannels);
    ASSERTSTR(frequency(0) <= frequency(nChannels - 1),
        "Channels are in reverse order. This is not supported yet.");
    // TODO: Technically, checking for equal channel widths is not enough,
    // because there could still be gaps between channels even though the
    // widths are all equal (this is not prevented by the MS 2.0 standard).
    ASSERTSTR(allEQ(width, width(0)),
        "Channels width is not the same for all channels. This is not supported"
        " yet.");

    double lower = frequency(0) - 0.5 * width(0);
    double upper = frequency(nChannels - 1) + 0.5 * width(nChannels - 1);
    regular_series series(lower, (upper - lower) / nChannels);
    itsSpectrum = cell_centered_axis<regular_series>(series, nChannels);
}


void MeasurementAIPS::initPolarizationInfo
    (const ROMSPolarizationColumns &polarization, uint id)
{
    ASSERT(polarization.nrow() > id);
    ASSERT(!polarization.flagRow()(id));

    Vector<Int> products = polarization.corrType()(id);

    itsPolarizations.resize(products.nelements());
    for(size_t i = 0; i < products.nelements(); ++i)
    {
        itsPolarizations[i] = Stokes::name(Stokes::type(products(i)));
    }
}


void MeasurementAIPS::initFieldInfo(const ROMSFieldColumns &field, uint id)
{
    /*
      Get phase center as RA and DEC (J2000).

      From AIPS++ note 229 (MeasurementSet definition version 2.0):
      ---
      FIELD: Field positions for each source
      Notes:
      The FIELD table defines a field position on the sky. For interferometers,
      this is the correlated field position. For single dishes, this is the
      nominal pointing direction.
      ---

      In LOFAR/CEP/BB/MS/src/makemsdesc.cc the following line can be found:
      MDirection phaseRef = mssubc.phaseDirMeasCol()(0)(IPosition(1,0));
      This should be equivalent to:
      MDirection phaseRef = mssubc.phaseDirMeas(0);
      as used in the code below.
    */
    ASSERT(field.nrow() > id);
    ASSERT(!field.flagRow()(id));
    itsPhaseCenter =
        MDirection::Convert(field.phaseDirMeas(id),MDirection::J2000)();
}

/*
void Measurement::readTimeInfo(const Table &selection)
{
    ROScalarColumn<double> time(selection, "TIME");
    ROScalarColumn<double> interval(selection, "INTERVAL");

    ASSERT(time.nrow() == interval.nrow() && time.nrow() > 0);

    uint nTimes = time.nrow();
    itsTimeRange[0] = time(0) - interval(0) / 2.0;
    itsTimeRange[1] = time(nTimes - 1) + interval(nTimes - 1) / 2.0;
}
*/


// OPTIMIZATION OPPORTUNITY: Cache implementation specific selection within
// a specialization of VisSelection or VisData.
TableExprNode
MeasurementAIPS::getTAQLExpression(const VisSelection &selection) const
{
    TableExprNode filter(true);

    const pair<double, double> &timeRange = selection.getTimeRange();
    if(selection.isSet(VisSelection::TIME_START))
    {
        filter = filter &&
            itsMS.col("TIME") >= timeRange.first;
    }

    if(selection.isSet(VisSelection::TIME_END))
    {
        filter = filter
            && itsMS.col("TIME") <= timeRange.second;
    }

    if(selection.isSet(VisSelection::STATIONS))
    {
        set<string> stations = selection.getStations();
        ASSERT(stations.size() > 0);

        set<size_t> selection;
        for(set<string>::const_iterator it = stations.begin();
            it != stations.end();
            ++it)
        {
            Regex regex = Regex::fromPattern(*it);

            // If the name of a station matches the pattern, add it to the
            // selection.
            for(size_t i = 0; i < itsInstrument.getStationCount(); ++i)
            {
                String name(itsInstrument.stations[i].name);
                if(name.matches(regex))
                {
                    selection.insert(i);
                }
            }
        }

        TableExprNodeSet selectionExpr;
        for(set<size_t>::const_iterator it = selection.begin();
            it != selection.end();
            ++it)
        {
            selectionExpr.add(TableExprNodeSetElem(static_cast<Int>(*it)));
        }

        filter = filter && itsMS.col("ANTENNA1").in(selectionExpr)
            && itsMS.col("ANTENNA2").in(selectionExpr);
    }

    if(selection.isSet(VisSelection::BASELINE_FILTER))
    {
        if(selection.getBaselineFilter() == VisSelection::AUTO)
            filter = filter && (itsMS.col("ANTENNA1") == itsMS.col("ANTENNA2"));
        else if(selection.getBaselineFilter() == VisSelection::CROSS)
            filter = filter && (itsMS.col("ANTENNA1") != itsMS.col("ANTENNA2"));
    }

    if(selection.isSet(VisSelection::POLARIZATIONS))
    {
        LOG_WARN_STR("Polarization selection not yet implemented; all available"
            " polarizations will be used.");
    }

    return filter;
}


// NOTE: Optimization opportunity: when reading all channels, do not use a
// slicer at all.
Slicer MeasurementAIPS::getCellSlicer(const VisSelection &selection) const
{
    // Construct slicer with default setting.
    IPosition start(2, 0, 0);
    size_t lastChannel = getChannelCount() - 1;
    size_t lastPolarization = getPolarizationCount() - 1;
    IPosition end(2, lastPolarization, lastChannel);

    // Validate and set selection.
    pair<size_t, size_t> range = selection.getChannelRange();
    if(selection.isSet(VisSelection::CHANNEL_START))
        start = IPosition(2, 0, range.first);

    if(selection.isSet(VisSelection::CHANNEL_END))
        if(range.second > lastChannel)
            LOG_WARN("Invalid end channel specified; using last channel"
                " instead.");
        else
            end = IPosition(2, lastPolarization, range.second);

    return Slicer(start, end, Slicer::endIsLast);
}


VisGrid
MeasurementAIPS::grid(const Table tab_selection, const Slicer slicer) const
{
    VisGrid grid;

    IPosition shape = slicer.length();
    size_t nPolarizations = shape[0];
    size_t nChannels = shape[1];

    // Extract time grid based on TIME column (mid-point of integration
    // interval).
    // TODO: Should use TIME_CENTROID here (centroid of exposure)?
    // NOTE: UVW is given of the TIME_CENTROID, not for TIME!
    // NOTE: TIME_CENTROID may be different for each baseline!
    ROScalarColumn<Double> c_time(tab_selection, "TIME");
    Vector<Double> time = c_time.getColumn();

    // Find all unique timeslots.
    Vector<uInt> timeIndex;
    uInt nTimeslots = GenSortIndirect<double>::sort(timeIndex, time,
        Sort::Ascending, Sort::InsSort + Sort::NoDuplicates);

    // Find all unique baselines.
    Block<String> sortColumns(2);
    sortColumns[0] = "ANTENNA1";
    sortColumns[1] = "ANTENNA2";
    Table baselines = tab_selection.sort(sortColumns, Sort::Ascending,
        Sort::QuickSort + Sort::NoDuplicates);
    uInt nBaselines = baselines.nrow();

    LOG_DEBUG_STR("Selection contains " << nBaselines << " baseline(s), "
        << nTimeslots << " timeslot(s), " << nChannels << " channel(s), and "
        << nPolarizations << " polarization(s).");

    // Initialize baseline axis.
    ROScalarColumn<Int> c_antenna1(baselines, "ANTENNA1");
    ROScalarColumn<Int> c_antenna2(baselines, "ANTENNA2");
    Vector<Int> antenna1 = c_antenna1.getColumn();
    Vector<Int> antenna2 = c_antenna2.getColumn();
    for(uInt i = 0; i < nBaselines; ++i)
        grid.baselines.push_back(baseline_t(antenna1[i], antenna2[i]));

    // Initialize time axis.
    ROScalarColumn<Double> c_interval(tab_selection, "INTERVAL");
    Vector<Double> interval = c_interval.getColumn();

    vector<double> times(nTimeslots + 1);
    for(uInt i = 0; i < nTimeslots; ++i)
        // Compute _lower border_ of each integration cell.
        times[i] = time[timeIndex[i]] - interval[timeIndex[i]] / 2.0;

    times[nTimeslots] = time[timeIndex[nTimeslots - 1]]
        + interval[timeIndex[nTimeslots - 1]] / 2.0;

    grid.time = cell_centered_axis<irregular_series>(irregular_series(times),
        nTimeslots);

    // Initialize frequency axis.
    double lower = itsSpectrum.lower(slicer.start()[1]);
    double upper = itsSpectrum.upper(slicer.end()[1]);
    double delta = (upper - lower) / nChannels;
    grid.freq = cell_centered_axis<regular_series>(regular_series(lower,
        delta), nChannels);

    // Initialize polarization axis.
    for(size_t i = 0; i < itsPolarizations.size(); ++i)
        grid.polarizations.push_back(itsPolarizations[i]);

    return grid;
}

} //# namespace BBS
} //# namespace LOFAR
