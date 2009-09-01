//# Equator.cc: Generate normal equations that tie a model to an observation.
//#
//# Copyright (C) 2008
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

#include <BBSKernel/Equator.h>
#include <BBSKernel/Exceptions.h>

#include <Common/lofar_iomanip.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

Equator::Equator(const VisData::Ptr &chunk, const Model::Ptr &model,
    const Grid &grid, const CoeffIndex &coeffIndex)
    :   itsChunk(chunk),
        itsModel(model),
        itsGrid(grid),
        itsIntersectionEmpty(true),
        itsSelectedCellCount(0)
{
    // By default, select all the baselines and polarizations products
    // available.
    const VisDimensions &dims = itsChunk->getDimensions();
    setSelection(dims.getBaselines(), dims.getPolarizations());

    // Create a mapping for each axis that maps from cells in the solution grid
    // to cell intervals in the observation (chunk) grid.
    makeGridMapping();

    // Create a mapping that maps each (parameter, coefficient) pair to an
    // index.
    makeCoeffMapping(coeffIndex);

    // Pre-allocate baseline processing buffers.
    itsBlContext.resize(coeffIndex.getCoeffCount());
}

void Equator::setSelection(const vector<baseline_t> &baselines,
    const vector<string> &products)
{
    itsBaselines = baselines;

    // Determine product mask.
    fill(itsProductMask, itsProductMask + 4, -1);

    const VisDimensions &dims = itsChunk->getDimensions();

    if(dims.hasPolarization("XX")
        && find(products.begin(), products.end(), "XX") != products.end())
    {
        itsProductMask[0] = dims.getPolarizationIndex("XX");
    }

    if(dims.hasPolarization("XY")
        && find(products.begin(), products.end(), "XY") != products.end())
    {
        itsProductMask[1] = dims.getPolarizationIndex("XY");
    }

    if(dims.hasPolarization("YX")
        && find(products.begin(), products.end(), "YX") != products.end())
    {
        itsProductMask[2] = dims.getPolarizationIndex("YX");
    }

    if(dims.hasPolarization("YY")
        && find(products.begin(), products.end(), "YY") != products.end())
    {
        itsProductMask[3] = dims.getPolarizationIndex("YY");
    }
}

void Equator::setCellSelection(const Location &start, const Location &end)
{
    // Check if [start, end] is a valid range.
    ASSERTSTR(start.first <= end.first && start.second <= end.second,
        "Invalid cell selection specified.");
    // Check if end location points to a valid cell in the solution grid.
    ASSERTSTR(end.first < itsGrid[FREQ]->size()
        && end.second < itsGrid[TIME]->size(),
        "Cell selection extends outside of solution grid.");

    // If the solution grid does not intersect the available visibility data
    // then the selection is necessarily empty. If the solution grid does
    // intersect the available visibility data but none of the cells in the
    // selection do, then the (effective) selection is empty as well.
    if(itsIntersectionEmpty
        || end.first < itsChunkStart.first
        || start.first > itsChunkEnd.first
        || end.second < itsChunkStart.second
        || start.second > itsChunkEnd.second)
    {
        itsSelectionStart = Location();
        itsSelectionEnd = Location();
        itsSelectedCellCount = 0;
    }
    else
    {
        // Clip the selection against the available visibility data.
        itsSelectionStart = Location(std::max(start.first, itsChunkStart.first),
            std::max(start.second, itsChunkStart.second));
        itsSelectionEnd = Location(std::min(end.first, itsChunkEnd.first),
             std::min(end.second, itsChunkEnd.second));
        itsSelectedCellCount =
            (itsSelectionEnd.first - itsSelectionStart.first + 1)
                * (itsSelectionEnd.second - itsSelectionStart.second + 1);

        LOG_DEBUG_STR("Cells to process (solution grid relative): [("
            << itsSelectionStart.first << "," << itsSelectionStart.second
            << "),(" << itsSelectionEnd.first << "," << itsSelectionEnd.second
            << ")]");

        // TODO: Construct Request instance and keep it (for caching).
        // TODO: Allocate #itsSelectedCellCount thread private LSQFit instances.
    }
}

void Equator::process(vector<CellEquation> &out)
{
    if(itsSelectedCellCount == 0)
    {
        return;
    }

    NSTimer timer;
    timer.start();

    // Allocate CellEquation instances (if necessary).
    out.resize(itsSelectedCellCount);

    // Initialize CellEquation instances.
    CellIterator cellIt(itsSelectionStart, itsSelectionEnd);
    for(size_t i = 0; i < out.size(); ++i)
    {
        // Set cell id (relative to the solution grid).
        out[i].id = itsGrid.getCellId(*cellIt);
        out[i].equation.set(static_cast<unsigned int>(getCoeffCount()));
        ++cellIt;
    }

    // Transform selection to chunk relative coordinates.
    const Location selStart(itsSelectionStart.first - itsChunkStart.first,
        itsSelectionStart.second - itsChunkStart.second);
    const Location selEnd(itsSelectionEnd.first - itsChunkStart.first,
        itsSelectionEnd.second - itsChunkStart.second);

    LOG_DEBUG_STR("Cells to process (chunk relative): [(" << selStart.first
        << "," << selStart.second << "),(" << selEnd.first << ","
        << selEnd.second << ")]");

    // Transform selection to the visbility (chunk) grid.
    const Location reqStart(itsFreqIntervals[selStart.first].start,
        itsTimeIntervals[selStart.second].start);
    const Location reqEnd(itsFreqIntervals[selEnd.first].end,
        itsTimeIntervals[selEnd.second].end);

    LOG_DEBUG_STR("Visibilities to process (chunk relative): [("
        << reqStart.first << "," << reqStart.second << "),(" << reqEnd.first
        << "," << reqEnd.second << ")]");

    // Set request grid.
    Grid reqGrid = itsChunk->getDimensions().getGrid().subset(reqStart, reqEnd);
    itsModel->setRequestGrid(reqGrid);

    // Process all baselines.
    BlContext &context = itsBlContext;
    context.reset();
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        blProcess(out.begin(), itsBaselines[i], selStart, selEnd, context);
    }
    timer.stop();

    LOG_DEBUG_STR("Processing speed: " << context.count / timer.getElapsed()
        << " vis/s");
    LOG_DEBUG_STR("No. of processed visibilities (unflagged): " << fixed
        << context.count);
    const double elapsed = timer.getElapsed() * 1e3;
    const unsigned long long count = timer.getCount();
    LOG_DEBUG_STR("Timer ms ALL total " << elapsed << " count " << count
        << " avg " << elapsed / count);
    for(size_t i = 0; i < Equator::BlContext::N_BlContextTimer; ++i)
    {
        const double elapsed = context.timers[i].getElapsed() * 1e3;
        const unsigned long long count = context.timers[i].getCount();
        LOG_DEBUG_STR("Timer ms " << Equator::BlContext::timerNames[i]
            << " total " << elapsed << " count " << count << " avg "
            << elapsed / count);
    }
}

void Equator::makeGridMapping()
{
    // Get reference to visibility grid.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();

    // Compute a mapping from cells of the solution grid to cell intervals in
    // the visibility grid.
    const pair<Interval, vector<Interval> > mapFreq =
        makeAxisMapping(itsGrid[FREQ], visGrid[FREQ]);

    const pair<Interval, vector<Interval> > mapTime =
        makeAxisMapping(itsGrid[TIME], visGrid[TIME]);

    // Store the mapping for each axis.
    itsFreqIntervals = mapFreq.second;
    itsTimeIntervals = mapTime.second;

    itsIntersectionEmpty = itsFreqIntervals.empty() || itsTimeIntervals.empty();
    if(itsIntersectionEmpty)
    {
        LOG_WARN_STR("Intersection of the solution grid and the current chunk"
            " is empty.");
    }

    // Store the indices of the first and last cell of the solution grid that
    // intersect the visibility grid.
    itsChunkStart = Location(mapFreq.first.start, mapTime.first.start);
    itsChunkEnd = Location(mapFreq.first.end, mapTime.first.end);
}

pair<Equator::Interval, vector<Equator::Interval> >
Equator::makeAxisMapping(const Axis::ShPtr &from, const Axis::ShPtr &to) const
{
    Interval domain;
    vector<Interval> mapping;

    const double overlapStart = std::max(from->start(), to->start());
    const double overlapEnd = std::min(from->end(), to->end());

    if(overlapStart >= overlapEnd || casa::near(overlapStart, overlapEnd))
    {
        return make_pair(domain, mapping);
    }

    domain.start = from->locate(overlapStart);
    domain.end = from->locate(overlapEnd, false, domain.start);

    // Intervals are inclusive by convention.
    const size_t nCells = domain.end - domain.start + 1;
    ASSERT(nCells >= 1);
    mapping.reserve(nCells);

    // Special case for the first domain cell: lower and possibly upper boundary
    // may be located outside of the overlap between the "from" and "to" axis.
    Interval interval;
    interval.start = to->locate(std::max(from->lower(domain.start),
        overlapStart));
    interval.end = to->locate(std::min(from->upper(domain.start), overlapEnd),
        false, interval.start);
    mapping.push_back(interval);

    for(size_t i = 1; i < nCells - 1; ++i)
    {
        interval.start = to->locate(from->lower(domain.start + i), true,
            interval.end);
        interval.end = to->locate(from->upper(domain.start + i), false,
            interval.end);
        mapping.push_back(interval);
    }

    if(nCells > 1)
    {
        // Special case for the last domain cell: upper boundary may be located
        // outside of the overlap between the "from" and "to" axis.
        interval.start = to->locate(from->lower(domain.end), true,
            interval.end);
        interval.end = to->locate(std::min(from->upper(domain.end), overlapEnd),
            false, interval.end);
        mapping.push_back(interval);
    }

    return make_pair(domain, mapping);
}

void Equator::makeCoeffMapping(const CoeffIndex &index)
{
    ParmGroup solvables = itsModel->getSolvableParms();
    ParmGroup::const_iterator solIt = solvables.begin();

    while(solIt != solvables.end())
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*solIt);

        CoeffIndex::const_iterator indexIt = index.find(parm->getName());
        ASSERT(indexIt != index.end());

        const CoeffInterval &interval = indexIt->second;
        for(unsigned int i = 0; i < interval.length; ++i)
        {
            itsCoeffMap[PValueKey(parm->getId(), i)] = interval.start + i;
        }

        ++solIt;
    }
}

// -------------------------------------------------------------------------- //
// - Equator::Interval implementation                                       - //
// -------------------------------------------------------------------------- //
Equator::Interval::Interval()
    :   start(0),
        end(0)
{
}

// -------------------------------------------------------------------------- //
// - Equator::BlContext implementation                                          - //
// -------------------------------------------------------------------------- //
Equator::BlContext::BlContext()
    :   count(0)
{
}

void Equator::BlContext::resize(unsigned int nCoeff)
{
    index.resize(nCoeff);
    partial.resize(nCoeff);
    partialRe.resize(nCoeff);
    partialIm.resize(nCoeff);
}

void Equator::BlContext::reset()
{
    count = 0;
    for(size_t i = 0; i < Equator::BlContext::N_BlContextTimer; ++i)
    {
        timers[i].reset();
    }
}

string Equator::BlContext::timerNames[Equator::BlContext::N_BlContextTimer] =
    {"MODEL_EVAL",
    "EQUATE",
    "BUILD_INDEX",
    "TRANSPOSE",
    "MAKE_NORM"};

} //# namespace BBS
} //# namespace LOFAR
