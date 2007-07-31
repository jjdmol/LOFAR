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
        :   itsFilename(filename),
            itsObservationId(observationId),
            itsDataDescriptionId(dataDescriptionId),
            itsFieldId(fieldId)
{
    itsMS = casa::MeasurementSet(itsFilename);

    itsInstrument = readInstrumentInfo(itsMS.antenna(), itsMS.observation(),
        itsObservationId);

    // Read time range.
    MSObservation observation(itsMS.observation());
    ROMSObservationColumns observationCols(observation);

    ASSERT(observation.nrow() > observationId);
    ASSERT(!observationCols.flagRow()(observationId));

    const Vector<MEpoch> timeRange =
        observationCols.timeRangeMeas()(observationId);

    itsTimeRange = make_pair(timeRange(0), timeRange(1));

    // Read polarization id and spectral window id.
    ROMSDataDescColumns dataDescription(itsMS.dataDescription());
    ASSERT(itsMS.dataDescription().nrow() > dataDescriptionId);
    ASSERT(!dataDescription.flagRow()(dataDescriptionId));

    Int polarizationId =
        dataDescription.polarizationId()(dataDescriptionId);

    Int spectralWindowId =
        dataDescription.spectralWindowId()(dataDescriptionId);

    itsCorrelationProducts =
        readPolarizationInfo(itsMS.polarization(), polarizationId);

    readSpectralInfo(itsMS.spectralWindow(), spectralWindowId);
    itsPhaseCenter = readFieldInfo(itsMS.field(), itsFieldId);

    // Select all rows that match the specified observation.
    itsMS = itsMS(itsMS.col("OBSERVATION_ID") == itsObservationId
        && itsMS.col("DATA_DESC_ID") == itsDataDescriptionId
        && itsMS.col("FIELD_ID") == itsFieldId);

    // Sort MS on TIME.
    itsMS.sort("TIME");

    LOG_DEBUG_STR("Measurement " << itsObservationId << " contains "
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

    return getGrid(tab_selection, slicer);
}


VisData::Pointer MeasurementAIPS::read(const VisSelection &selection,
    const string &column,
    bool readUVW) const
{
    NSTimer readTimer, copyTimer;

    TableExprNode taqlExpr = getTAQLExpression(selection);
    Slicer slicer = getCellSlicer(selection);

    Table tab_selection = itsMS(taqlExpr);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection empty!");

    VisGrid grid = getGrid(tab_selection, slicer);
    VisData::Pointer buffer = allocate(grid);

    size_t nChannels = buffer->freq.size();
    size_t nCorrelations = getCorrelationCount();

    TableIterator tslotIterator(tab_selection, "TIME");

    Table tslot;
    uint32 tslotIdx = 0;
    while(!tslotIterator.pastEnd())
    {
        ASSERT(tslotIdx < buffer->getTimeslotCount());

        // Get next timeslot.
        tslot = tslotIterator.table();

        // Declare all the columns we want to read.
        ROScalarColumn<Int> c_antenna1(tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tslot, "ANTENNA2");
        ROScalarColumn<Bool> c_flag_row(tslot, "FLAG_ROW");
        ROArrayColumn<Double> c_uvw(tslot, "UVW");
        ROArrayColumn<Complex> c_data(tslot, column);
        ROArrayColumn<Bool> c_flag(tslot, "FLAG");

        // Read data from the MS.
        readTimer.start();
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        Vector<Bool> aips_flag_row = c_flag_row.getColumn();
        Matrix<Double> aips_uvw;
        if(readUVW)
             aips_uvw = c_uvw.getColumn();

        Cube<Complex> aips_data = c_data.getColumn(slicer);
        Cube<Bool> aips_flag = c_flag.getColumn(slicer);
        readTimer.stop();

        // Validate shapes.
        ASSERT(aips_antenna1.shape().isEqual(IPosition(1,tslot.nrow())));
        ASSERT(aips_antenna2.shape().isEqual(IPosition(1,tslot.nrow())));
        ASSERT(aips_flag_row.shape().isEqual(IPosition(1, tslot.nrow())));
        ASSERT(!readUVW
            || aips_uvw.shape().isEqual(IPosition(2, 3, tslot.nrow())));
        IPosition shape = IPosition(3, nCorrelations, nChannels, tslot.nrow());
        ASSERT(aips_data.shape().isEqual(shape));
        ASSERT(aips_flag.shape().isEqual(shape));

        ASSERT(aips_data.contiguousStorage());
        ASSERT(aips_flag.contiguousStorage());

        // The const_cast<>() call below is needed becase multi_array_ref
        // expects a non-const pointer. This does not break the const semantics
        // as we never write to the array.
        Complex *ptr_data = const_cast<Complex*>(aips_data.data());
        boost::multi_array_ref<sample_t, 3>
            data(reinterpret_cast<sample_t*>(ptr_data),
                boost::extents[tslot.nrow()][nChannels][nCorrelations]);

        Bool *ptr_flag = const_cast<Bool*>(aips_flag.data());
        boost::multi_array_ref<flag_t, 3> flag(ptr_flag,
                boost::extents[tslot.nrow()][nChannels][nCorrelations]);

        copyTimer.start();
        for(uInt i = 0; i < tslot.nrow(); ++i)
        {
            // Get time sequence for this baseline.
            map<baseline_t, size_t>::iterator it =
                buffer->baselines.find(baseline_t(aips_antenna1[i],
                    aips_antenna2[i]));

            ASSERTSTR(it != buffer->baselines.end(), "Unknown baseline!");
            size_t baseline = it->second;

            // Flag timeslot as available.
            buffer->tslot_flag[baseline][tslotIdx] = 0;

            // Copy row (timeslot) flag.
            if(aips_flag_row(i))
                buffer->tslot_flag[baseline][tslotIdx] |=
                    VisData::FLAGGED_IN_INPUT;

            // Copy UVW.
            if(readUVW)
            {
                buffer->uvw[baseline][tslotIdx][0] = aips_uvw(0, i);
                buffer->uvw[baseline][tslotIdx][1] = aips_uvw(1, i);
                buffer->uvw[baseline][tslotIdx][2] = aips_uvw(2, i);
            }

            // Copy visibilities.
            buffer->vis_data[baseline][tslotIdx] = data[i];
            // Copy flags.
            buffer->vis_flag[baseline][tslotIdx] = flag[i];
        }
        copyTimer.stop();

        // Proceed to the next timeslot.
        ++tslotIdx;
        ++tslotIterator;
    }

    LOG_DEBUG_STR("Read time: " << readTimer);
    LOG_DEBUG_STR("Copy time: " << copyTimer);

    return buffer;
}



void MeasurementAIPS::write(const VisSelection &selection,
    VisData::Pointer buffer,
    const string &column,
    bool writeFlags) const
{
    NSTimer readTimer, writeTimer;

    ASSERTSTR(!writeFlags, "Flag writing not implemented yet!");

    // TODO: The addition of a column is not noted by the MeasurementSet
    // class.
    Table out(itsFilename);
    out.reopenRW();

    Slicer slicer = getCellSlicer(selection);
    size_t nChannels = buffer->freq.size();
    size_t nCorrelations = getCorrelationCount();

    // Add column if it does not exist.
    // TODO: Check why the AIPS++ imager does not recognize these columns.
    if(!out.tableDesc().isColumn(column))
    {
        LOG_INFO_STR("Adding column '" << column << "'.");

        ArrayColumnDesc<Complex> descriptor(column,
            IPosition(2, nCorrelations, nChannels),
            ColumnDesc::FixedShape);
        TiledColumnStMan storageManager("Tiled_" + column,
            IPosition(3, nCorrelations, nChannels, 1));

        out.addColumn(descriptor, storageManager);
        out.flush();
    }

    LOG_DEBUG_STR("Writing to column: " << column);

    TableExprNode taqlExpr = getTAQLExpression(selection);
    Table tab_selection = itsMS(taqlExpr);
    ASSERTSTR(tab_selection.nrow() > 0, "Data selection empty!");

    Table tslot;
    size_t tslotIdx = 0;
    bool mismatch = false;
    TableIterator tslotIterator(tab_selection, "TIME");
    while(!tslotIterator.pastEnd())
    {
        ASSERTSTR(tslotIdx < buffer->getTimeslotCount(),
            "Timeslot out of range!");

        // Extract next timeslot.
        tslot = tslotIterator.table();
//        LOG_DEBUG_STR("No. of rows in timeslot " << tslotIdx << ": "
//            << tslot.nrow());

        // TODO: Should use TIME_CENTROID here (centroid of exposure)?
        // NOTE: TIME_CENTROID may be different for each baseline!
        ROScalarColumn<Double> c_time(tslot, "TIME");
        ROScalarColumn<Int> c_antenna1(tslot, "ANTENNA1");
        ROScalarColumn<Int> c_antenna2(tslot, "ANTENNA2");
        ArrayColumn<Complex> c_data(tslot, column);
        ArrayColumn<Bool> c_flag(tslot, "FLAG");

        // Read meta data.
        readTimer.start();
        Vector<Double> aips_time = c_time.getColumn();
        Vector<Int> aips_antenna1 = c_antenna1.getColumn();
        Vector<Int> aips_antenna2 = c_antenna2.getColumn();
        readTimer.stop();

        // Validate shapes.
        ASSERT(aips_antenna1.shape().isEqual(IPosition(1, tslot.nrow())));
        ASSERT(aips_antenna2.shape().isEqual(IPosition(1, tslot.nrow())));
        ASSERT(aips_time.shape().isEqual(IPosition(1, tslot.nrow())));

        mismatch = mismatch && (buffer->time(tslotIdx) == aips_time(0));

        writeTimer.start();
        for(uInt i = 0; i < tslot.nrow(); ++i)
        {
            // Get time sequence for this baseline.
            map<baseline_t, size_t>::iterator it =
                buffer->baselines.find(baseline_t(aips_antenna1[i],
                    aips_antenna2[i]));

            ASSERTSTR(it != buffer->baselines.end(), "Unknown baseline!");
            size_t baseline = it->second;

            // Write visibilities.
            Array<Complex> vis_data(IPosition(2, nCorrelations, nChannels),
                reinterpret_cast<Complex*>
                    (&(buffer->vis_data[baseline][tslotIdx][0][0])),
                SHARE);
            c_data.putSlice(i, slicer, vis_data);
        }
        writeTimer.stop();

        ++tslotIdx;
        ++tslotIterator;
    }
    tab_selection.flush();

    if(mismatch)
        LOG_WARN_STR("Time mismatches detected while writing data.");

    LOG_DEBUG_STR("Read time (meta data): " << readTimer);
    LOG_DEBUG_STR("Write time: " << writeTimer);
}


Instrument
MeasurementAIPS::readInstrumentInfo(const casa::MSAntenna &tab_antenna,
    const casa::MSObservation &tab_observation,
    uint id)
{
    // Get station names and positions in ITRF coordinates.
    ROMSAntennaColumns antenna(tab_antenna);
    ROMSObservationColumns observation(tab_observation);
    ASSERT(tab_observation.nrow() > id);
    ASSERT(!observation.flagRow()(id));

    Instrument instrument;
    instrument.name = observation.telescopeName()(id);
    instrument.stations.resize(tab_antenna.nrow());

    MVPosition stationCentroid;
    for(size_t i = 0; i < instrument.stations.size(); ++i)
    {
        // Store station name and update index.
        instrument.stations[i].name = antenna.name()(i);
        instrument.stationIndex[antenna.name()(i)] = i;

        // Store station position.
        MPosition position = antenna.positionMeas()(i);
        instrument.stations[i].position =
            MPosition::Convert(position, MPosition::ITRF)();

        // Update centroid.
        stationCentroid += instrument.stations[i].position.getValue();
    }

    // Get the instrument position in ITRF coordinates, or use the centroid
    // of the station positions if the instrument position is unknown.
    MPosition instrumentPosition;
    if(MeasTable::Observatory(instrumentPosition, instrument.name))
    {
        instrument.position =
            MPosition::Convert(instrumentPosition, MPosition::ITRF)();
    }
    else
    {
        LOG_WARN("Instrument position unknown; will use centroid of stations.");
        stationCentroid *= (1.0 / (double) instrument.stations.size());
        instrument.position = MPosition(stationCentroid, MPosition::ITRF);
    }

    return instrument;
}

void MeasurementAIPS::readSpectralInfo(const casa::MSSpectralWindow &tab_window,
    uint id)
{
    ROMSSpWindowColumns window(tab_window);
    ASSERT(tab_window.nrow() > id);
    ASSERT(!window.flagRow()(id));

    Int nChannels = window.numChan()(id);
    ASSERT(nChannels > 0);

    Vector<Double> frequency = window.chanFreq()(id);
    Vector<Double> width = window.chanWidth()(id);

    ASSERT(frequency.nelements() == nChannels);
    ASSERT(width.nelements() == nChannels);
    ASSERTSTR(frequency(0) <= frequency(nChannels - 1),
        "Channels are in reverse order. This is not supported yet.");
    // TODO: Technically, checking for equal channel widths is not enough,
    // because there could still be gaps between channels even though the
    // widths are still equal (this is not prevented by the MS 2.0 standard).
    ASSERTSTR(allEQ(width, width(0)),
        "Channels width is not the same for all channels. This is not supported"
        " yet.");

    double lower = frequency(0) - 0.5 * width(0);
    double upper = frequency(nChannels - 1) + 0.5 * width(nChannels - 1);
    regular_series series(lower, (upper - lower) / nChannels);
    itsSpectrum = cell_centered_axis<regular_series>(series, nChannels);
}


vector<string>
MeasurementAIPS::readPolarizationInfo(const MSPolarization &tab_polarization,
    uint id)
{
    ROMSPolarizationColumns polarization(tab_polarization);
    ASSERT(tab_polarization.nrow() > id);
    ASSERT(!polarization.flagRow()(id));

    Vector<Int> products = polarization.corrType()(id);

    vector<string> result;
    for(size_t i = 0; i < products.nelements(); ++i)
        result.push_back(Stokes::name(Stokes::type(products(i))));

    return result;
}


casa::MDirection MeasurementAIPS::readFieldInfo(const MSField &tab_field,
    uint id)
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
    ROMSFieldColumns field(tab_field);
    ASSERT(tab_field.nrow() > id);
    ASSERT(!field.flagRow()(id));

    return MDirection::Convert(field.phaseDirMeas(id), MDirection::J2000)();
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
            casa::Regex regex = casa::Regex::fromPattern(*it);

            // If the name of a station matches the pattern, add it to the
            // selection.
            for(size_t i = 0; i < itsInstrument.getStationCount(); ++i)
            {
                casa::String name(itsInstrument.stations[i].name);
                if(name.matches(regex))
                    selection.insert(i);
            }
        }

        TableExprNodeSet selectionExpr;
        for(set<size_t>::const_iterator it = selection.begin();
            it != selection.end();
            ++it)
        {
            selectionExpr.add(
                TableExprNodeSetElem(static_cast<casa::Int>(*it)));
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

    if(selection.isSet(VisSelection::CORRELATIONS))
    {
        LOG_WARN_STR("Correlation selection not yet implemented; all available"
            " correlations will be used.");
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
    size_t lastCorrelation = getCorrelationCount() - 1;
    IPosition end(2, lastCorrelation, lastChannel);

    // Validate and set selection.
    pair<size_t, size_t> range = selection.getChannelRange();
    if(selection.isSet(VisSelection::CHANNEL_START))
        start = IPosition(2, 0, range.first);

    if(selection.isSet(VisSelection::CHANNEL_END))
        if(range.second > lastChannel)
            LOG_WARN("Invalid end channel specified; using last channel"
                " instead.");
        else
            end = IPosition(2, lastCorrelation, range.second);

    return Slicer(start, end, Slicer::endIsLast);
}


VisGrid MeasurementAIPS::getGrid(const casa::Table tab_selection,
    const casa::Slicer slicer) const
{
    VisGrid grid;

    IPosition shape = slicer.length();
    size_t nCorrelations = shape[0];
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
        << nCorrelations << " correlation(s).");

    // Initialize baseline axis.
    ROScalarColumn<Int> c_antenna1(baselines, "ANTENNA1");
    ROScalarColumn<Int> c_antenna2(baselines, "ANTENNA2");
    Vector<Int> antenna1 = c_antenna1.getColumn();
    Vector<Int> antenna2 = c_antenna2.getColumn();
    for(uInt i = 0; i < nBaselines; ++i)
        grid.baselines.insert(baseline_t(antenna1[i], antenna2[i]));

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
    for(size_t i = 0; i < itsCorrelationProducts.size(); ++i)
        grid.polarizations.insert(itsCorrelationProducts[i]);

    return grid;
}


VisData::Pointer MeasurementAIPS::allocate(const VisGrid &grid) const
{
    size_t nTimeslots = grid.time.size();
    size_t nBaselines = grid.baselines.size();
    size_t nChannels = grid.freq.size();
    size_t nPolarizations = grid.polarizations.size();

    LOG_DEBUG_STR("Allocate: " << nBaselines << " baseline(s), "
        << nTimeslots << " timeslot(s), " << nChannels << " channel(s), and "
        << nPolarizations << " polarization(s).");

    VisData::Pointer buffer(new VisData(nTimeslots, nBaselines, nChannels,
        nPolarizations));

    // Initially flag all timeslots as UNAVAILABLE.
    for(size_t i = 0; i < nBaselines; ++i)
        for(size_t j = 0; j < nTimeslots; ++j)
           buffer->tslot_flag[i][j] = VisData::UNAVAILABLE;

    // Copy time and frequency axis.
    buffer->time = grid.time;
    buffer->freq = grid.freq;

    size_t index;

    // Initialize baseline index.
    index = 0;
    for(set<baseline_t>::const_iterator it = grid.baselines.begin();
        it != grid.baselines.end();
        ++it)
    {
        buffer->baselines[*it] = index;
        ++index;
    }

    // Initialize polarizations index.
    index = 0;
    for(set<string>::const_iterator it = grid.polarizations.begin();
        it != grid.polarizations.end();
        ++it)
    {
        buffer->polarizations[*it] = index;
        ++index;
    }

    return buffer;
}


} //# namespace BBS
} //# namespace LOFAR
