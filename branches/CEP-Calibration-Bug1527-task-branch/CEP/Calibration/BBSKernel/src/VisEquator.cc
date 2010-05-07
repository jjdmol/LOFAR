//# VisEquator.cc: Generate condition equations based on a buffer of observed
//# and a buffer of simulated visibilities.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/VisEquator.h>
#include <BBSKernel/Exceptions.h>

#include <Common/lofar_iomanip.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

VisEquator::VisEquator(const VisData::Ptr &lhs, const MeasurementExpr::Ptr &rhs)
    :   itsLHS(lhs),
        itsRHS(rhs),
        itsIntersectionEmpty(true),
        itsSelectedCellCount(0),
        itsCoeffCount(0)
{
    // Create a mapping for each axis that maps from cells in the measurement
    // grid to cells in the solution grid.
    makeCellMap();

    // Construct a sequence of pairs of indices of matching baselines (i.e.
    // baselines known by both LHS and RHS).
    makeIndexMap(itsLHS->baselines(), itsRHS->baselines(),
        back_inserter(itsBlMap));

    // Construct a sequence of pairs of indices of matching correlations (i.e.
    // correlations known by both LHS and RHS).
    makeIndexMap(itsLHS->correlations(), itsRHS->correlations(),
        back_inserter(itsCrMap));

    // By default select all the cells in the solution grid the intersect the
    // evaluation grid for processing.
    setCellSelection(itsEvalStart, itsEvalEnd);
}

VisEquator::~VisEquator()
{
    itsRHS->clearSolvables();
}

void VisEquator::setSolutionGrid(const Grid &grid)
{
    itsSolGrid = grid;

    // Regenerate solution cell map.
    makeCellMap();

    // By default select all the cells in the solution grid the intersect the
    // evaluation grid for processing.
    setCellSelection(itsEvalStart, itsEvalEnd);
}

void VisEquator::setCellSelection(const Location &start, const Location &end)
{
    // Check that [start, end] is a valid range.
    ASSERTSTR(start.first <= end.first && start.second <= end.second,
        "Invalid cell selection specified (start > end).");
    // Check if end location points to a valid cell in the solution grid.
    ASSERTSTR(end.first < itsSolGrid[FREQ]->size()
        && end.second < itsSolGrid[TIME]->size(),
        "Cell selection extends outside of the solution grid.");

    // If the solution grid does not intersect the evaluation grid then the
    // selection is empty. If the solution grid does intersect the evaluation
    // grid but none of the cells in the selection do, then the (effective)
    // selection is empty as well.
    if(itsIntersectionEmpty
        || end.first < itsEvalStart.first
        || start.first > itsEvalEnd.first
        || end.second < itsEvalStart.second
        || start.second > itsEvalEnd.second)
    {
        itsSelectionStart = itsSelectionEnd = Location();
        itsSelectedCellCount = 0;
        itsEvalSelStart = itsEvalSelEnd = Location();
        itsEvalReqStart = itsEvalReqEnd = Location();
        itsReqStart = itsReqEnd = Location();
    }
    else
    {
        // Clip the selection against the evaluation grid.
        itsSelectionStart = Location(std::max(start.first, itsEvalStart.first),
            std::max(start.second, itsEvalStart.second));
        itsSelectionEnd = Location(std::min(end.first, itsEvalEnd.first),
             std::min(end.second, itsEvalEnd.second));
        itsSelectedCellCount =
            (itsSelectionEnd.first - itsSelectionStart.first + 1)
                * (itsSelectionEnd.second - itsSelectionStart.second + 1);

        LOG_DEBUG_STR("Solution cells to process (solution grid coordinates):"
            " [(" << itsSelectionStart.first << "," << itsSelectionStart.second
            << "),(" << itsSelectionEnd.first << "," << itsSelectionEnd.second
            << ")]");

        // Translate the selection to coordinates relative to the start of the
        // observation grid.
        itsEvalSelStart = Location(itsSelectionStart.first - itsEvalStart.first,
            itsSelectionStart.second - itsEvalStart.second);
        itsEvalSelEnd = Location(itsSelectionEnd.first - itsEvalStart.first,
            itsSelectionEnd.second - itsEvalStart.second);

        // Transform the selection to evaluation grid coordinates.
        itsEvalReqStart.first = distance(itsFreqMap.begin(),
            lower_bound(itsFreqMap.begin(), itsFreqMap.end(),
            itsEvalSelStart.first));
        itsEvalReqStart.second = distance(itsTimeMap.begin(),
            lower_bound(itsTimeMap.begin(), itsTimeMap.end(),
            itsEvalSelStart.second));

        itsEvalReqEnd.first = distance(itsFreqMap.begin(),
            upper_bound(itsFreqMap.begin(), itsFreqMap.end(),
            itsEvalSelEnd.first)) - 1;
        itsEvalReqEnd.second = distance(itsTimeMap.begin(),
            upper_bound(itsTimeMap.begin(), itsTimeMap.end(),
                itsEvalSelEnd.second)) - 1;

        // Transform the selection to observation grid coordinates.
        itsReqStart.first = itsEvalOffset.first + itsEvalReqStart.first;
        itsReqStart.second = itsEvalOffset.second + itsEvalReqStart.second;

        itsReqEnd.first = itsEvalOffset.first + itsEvalReqEnd.first;
        itsReqEnd.second = itsEvalOffset.second + itsEvalReqEnd.second;

        LOG_DEBUG_STR("Samples to process (observation grid coordinates): [("
            << itsReqStart.first << "," << itsReqStart.second << "),("
            << itsReqEnd.first << "," << itsReqEnd.second << ")]");

        Grid reqGrid = itsLHS->grid().subset(itsReqStart, itsReqEnd);
        itsRHS->setEvalGrid(reqGrid);
    }
}

size_t VisEquator::nSelectedCells() const
{
    return itsSelectedCellCount;
}

bool VisEquator::isSelectionEmpty() const
{
    return itsIntersectionEmpty || itsBlMap.empty() || itsCrMap.empty();
}

ParmGroup VisEquator::parms() const
{
    return itsRHS->parms();
}

ParmGroup VisEquator::solvables() const
{
    return itsRHS->solvables();
}

void VisEquator::setSolvables(const ParmGroup &solvables)
{
    // Notify measurement expression of changed solvable selection.
    itsRHS->setSolvables(solvables);

    // Regenerate coefficient map.
    makeCoeffMap();

    // Pre-allocate buffers for processing.
    itsProcContext.resize(itsCoeffCount);
}

void VisEquator::solvablesChanged()
{
    itsRHS->solvablesChanged();
}

void VisEquator::clearStats()
{
    itsProcTimer.reset();
    itsProcContext.clearStats();
}

void VisEquator::dumpStats(ostream &out) const
{
    const NSTimer &timer = itsProcTimer;
    const ProcContext &context = itsProcContext;

    out << "Processing speed: " << context.count / timer.getElapsed()
        << " samples/s" << endl;
    out << "No. of processed samples (unflagged): " << fixed << context.count
        << endl;
    out << "TIMER s ALL total " << timer.getElapsed() << " count "
        << timer.getCount() << " avg " << timer.getElapsed()
        / timer.getCount() << endl;

    for(size_t i = 0; i < VisEquator::ProcContext::N_ProcTimer; ++i)
    {
        const double elapsed = context.timers[i].getElapsed();
        const unsigned long long count = context.timers[i].getCount();

        out << "TIMER s " << VisEquator::ProcContext::timerNames[i] << " total"
            << " " << elapsed << " count " << count << " avg " << elapsed
            / count << endl;
    }
}

void VisEquator::setBaselineMask(const BaselineMask &mask)
{
    itsBlMap.clear();
    makeIndexMap(itsLHS->baselines(), itsRHS->baselines(), mask,
        back_inserter(itsBlMap));
}

void VisEquator::setCorrelationMask(const CorrelationMask &mask)
{
    itsCrMap.clear();
    makeIndexMap(itsLHS->correlations(), itsRHS->correlations(), mask,
        back_inserter(itsCrMap));
}

Interval<size_t> VisEquator::findContainedCellRange(const Axis::ShPtr &axis,
    const Interval<double> &interval) const
{
    Interval<double> overlap(std::max(axis->start(), interval.start),
        std::min(axis->end(), interval.end));

    if(overlap.start >= overlap.end || casa::near(overlap.start, overlap.end))
    {
        return Interval<size_t>(1, 0);
    }

    size_t start = axis->locate(overlap.start);
    size_t end = axis->locate(overlap.end, false, start);
    ASSERT(start <= end);

    // Check for special case: start cell is not completely contained in the
    // provided interval.
    if(!casa::near(axis->lower(start), overlap.start))
    {
        if(start == end)
        {
            return Interval<size_t>(1, 0);
        }

        ++start;
    }

    // Check for special case: end cell is not completely contained in the
    // provided interval.
    if(!casa::near(axis->upper(end), overlap.end))
    {
        if(end == start)
        {
            return Interval<size_t>(1, 0);
        }

        --end;
    }

    ASSERT(start <= end);
    return Interval<size_t>(start, end);
}

void VisEquator::makeEvalGrid()
{
    // Clear information that needs to be regenerated.
    itsIntersectionEmpty = true;
    itsEvalGrid = Grid();
    itsEvalOffset = Location();

    // Find the range of cells on each axis of the observation grid that is
    // completely contained within the model domain.
    Box domainRHS = itsRHS->domain();
    Interval<size_t> freqCellRange =
        findContainedCellRange(itsLHS->grid()[FREQ],
            Interval<double>(domainRHS.lowerX(), domainRHS.upperX()));
    Interval<size_t> timeCellRange =
        findContainedCellRange(itsLHS->grid()[TIME],
            Interval<double>(domainRHS.lowerY(), domainRHS.upperY()));

//    LOG_DEBUG_STR("freqCellRange: [" << freqCellRange.start << ","
//        << freqCellRange.end << "]");
//    LOG_DEBUG_STR("timeCellRange: [" << timeCellRange.start << ","
//        << timeCellRange.end << "]");

    // Check for empty intersection between observation grid and model domain.
    if(freqCellRange.start > freqCellRange.end
        || timeCellRange.start > timeCellRange.end)
    {
        return;
    }

    Axis::ShPtr freqAxis = itsLHS->grid()[FREQ]->subset(freqCellRange.start,
        freqCellRange.end);
    Axis::ShPtr timeAxis = itsLHS->grid()[TIME]->subset(timeCellRange.start,
        timeCellRange.end);

    itsIntersectionEmpty = false;
    itsEvalGrid = Grid(freqAxis, timeAxis);
    itsEvalOffset = Location(freqCellRange.start, timeCellRange.start);
}

void VisEquator::makeCellMap()
{
    // Clear information that needs to be regenerated.
    itsFreqMap.clear();
    itsTimeMap.clear();
    itsEvalStart = itsEvalEnd = Location();

    // Determine the evaluation grid, i.e. the part of the observation grid
    // that is completely contained in the model domain.
    makeEvalGrid();

    // Check for empty intersection between the observation grid and the model
    // domain.
    if(itsIntersectionEmpty)
    {
        return;
    }

    // Compute a mapping from cells of the solution grid to cell intervals in
    // the evaluation grid.
    Interval<size_t> freqDomain = makeAxisMap(itsSolGrid[FREQ],
        itsEvalGrid[FREQ], back_inserter(itsFreqMap));
    Interval<size_t> timeDomain = makeAxisMap(itsSolGrid[TIME],
        itsEvalGrid[TIME], back_inserter(itsTimeMap));

    // Check for empty intersection between evaluation grid and solution grid.
    itsIntersectionEmpty = itsFreqMap.empty() || itsTimeMap.empty();
    if(itsIntersectionEmpty)
    {
        return;
    }

    // Store the indices of the first and last cell of the solution grid that
    // intersect the evaluation grid.
    itsEvalStart = Location(freqDomain.start, timeDomain.start);
    itsEvalEnd = Location(freqDomain.end, timeDomain.end);

//    LOG_DEBUG_STR("Domain: " << freqDomain.start << " " << freqDomain.end);
//    LOG_DEBUG_STR("Map: " << itsFreqMap);
//    LOG_DEBUG_STR("Domain: " << timeDomain.start << " " << timeDomain.end);
//    LOG_DEBUG_STR("Map: " << itsTimeMap);
}

void VisEquator::makeCoeffMap()
{
    // Clear information that needs to be regenerated.
    itsCoeffCount = 0;
    itsCoeffMap.clear();

    size_t index = 0;
    ParmGroup solvables = this->solvables();
    for(ParmGroup::const_iterator sol_it = solvables.begin(),
        sol_end = solvables.end(); sol_it != sol_end; ++sol_it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*sol_it);
        const size_t id = parm->getId();
        const size_t count = parm->getCoeffCount();

        itsCoeffCount += count;
        for(size_t i = 0; i < count; ++i)
        {
            itsCoeffMap[PValueKey(id, i)] = index++;
        }
    }
}

void VisEquator::makeElementCoeffMap(const ValueSet &element,
    ProcContext &context)
{
    size_t nCoeff = 0;
    for(ValueSet::const_iterator it = element.begin(), end = element.end();
        it != end; ++it)
    {
        // Look-up coefficient index for this coefficient.
        context.index[nCoeff] = itsCoeffMap[it->first];

        // Store a reference to the partial derivarive with respect to this
        // coefficient.
        context.partial[nCoeff] = it->second;

        // Update number of coefficients.
        ++nCoeff;
    }

    // Update the number of coefficients.
    context.nCoeff = nCoeff;
}

// -------------------------------------------------------------------------- //
// - VisEquator::ProcContext implementation                                 - //
// -------------------------------------------------------------------------- //
VisEquator::ProcContext::ProcContext()
    :   count(0)
{
}

void VisEquator::ProcContext::resize(size_t nCoeff)
{
    index.resize(nCoeff);
    partial.resize(nCoeff);
    partialRe.resize(nCoeff);
    partialIm.resize(nCoeff);
}

void VisEquator::ProcContext::clearStats()
{
    count = 0;
    for(size_t i = 0; i < VisEquator::ProcContext::N_ProcTimer; ++i)
    {
        timers[i].reset();
    }
}

string
VisEquator::ProcContext::timerNames[VisEquator::ProcContext::N_ProcTimer] =
    {"EVAL_EXPR",
    "EQUATE",
    "MAKE_COEFF_MAP",
    "TRANSPOSE",
    "MAKE_NORM"};

} //# namespace BBS
} //# namespace LOFAR
