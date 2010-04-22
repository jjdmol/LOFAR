//# Equator.cc: Generate normal equations that tie a model to an observation.
//#
//# Copyright (C) 2008
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

Equator::Equator(const ExprSet<JonesMatrix>::Ptr &lhs,
    const ExprSet<JonesMatrix>::Ptr &rhs, const Grid &evalGrid,
    const Grid &solGrid, const CoeffIndex &coeffIndex)
    :   itsLHS(lhs),
        itsRHS(rhs),
        itsEvalGrid(evalGrid),
        itsSolGrid(solGrid),
        itsIntersectionEmpty(true),
        itsSelectedCellCount(0)
{
    ASSERT(lhs->size() == rhs->size());
    ASSERT(lhs->domain().contains(itsEvalGrid.getBoundingBox()));
    ASSERT(rhs->domain().contains(itsEvalGrid.getBoundingBox()));

    // Create a mapping for each axis that maps from cells in the solution grid
    // to cell intervals in the evaluation grid.
    makeGridMapping();

    // Create a mapping that maps each (parameter, coefficient) pair to an
    // index.
    makeCoeffMapping(coeffIndex);

    // Pre-allocate expression processing buffers.
    itsProcContext.resize(coeffIndex.getCoeffCount());
}

void Equator::setCellSelection(const Location &start, const Location &end)
{
    // Check if [start, end] is a valid range.
    ASSERTSTR(start.first <= end.first && start.second <= end.second,
        "Invalid cell selection specified.");
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
        // evaluation grid.
        itsEvalSelStart = Location(itsSelectionStart.first - itsEvalStart.first,
            itsSelectionStart.second - itsEvalStart.second);
        itsEvalSelEnd = Location(itsSelectionEnd.first - itsEvalStart.first,
            itsSelectionEnd.second - itsEvalStart.second);

        // Transform the selection to evaluation grid coordinates.
        itsReqStart = Location(itsFreqIntervals[itsEvalSelStart.first].start,
            itsTimeIntervals[itsEvalSelStart.second].start);
        itsReqEnd = Location(itsFreqIntervals[itsEvalSelEnd.first].end,
            itsTimeIntervals[itsEvalSelEnd.second].end);

        LOG_DEBUG_STR("Samples to process (evaluation grid coordinates): [("
            << itsReqStart.first << "," << itsReqStart.second << "),("
            << itsReqEnd.first << "," << itsReqEnd.second << ")]");

        Grid evalGrid = itsEvalGrid.subset(itsReqStart, itsReqEnd);
        itsLHS->setEvalGrid(evalGrid);
        itsRHS->setEvalGrid(evalGrid);

        // TODO: Allocate #itsSelectedCellCount thread private LSQFit instances.
    }
}

void Equator::process(vector<CellEquation> &out)
{
    // Allocate memory for CellEquation instances (if necessary).
    out.resize(itsSelectedCellCount);

    if(itsSelectedCellCount == 0)
    {
        return;
    }

    NSTimer timer;
    timer.start();

    // Initialize CellEquation instances.
    CellIterator cellIt(itsSelectionStart, itsSelectionEnd);
    for(size_t i = 0; i < out.size(); ++i)
    {
        // Set cell id (relative to the solution grid).
        out[i].id = itsSolGrid.getCellId(*cellIt);
        out[i].equation.set(static_cast<unsigned int>(itsCoeffMap.size()));
        ++cellIt;
    }

    // Process all expressions in the set.
    ProcContext &context = itsProcContext;
    context.reset();
    for(size_t idx = 0, size = itsLHS->size(); idx < size; ++idx)
    {
        procExpr(out.begin(), idx, context);
    }
    timer.stop();

    LOG_DEBUG_STR("Processing speed: " << context.count / timer.getElapsed()
        << " samples/s");
    LOG_DEBUG_STR("No. of processed samples (unflagged): " << fixed
        << context.count);
    LOG_DEBUG_STR("TIMER ms ALL total " << timer.getElapsed() * 1e3 << " count "
        << timer.getCount() << " avg " << (timer.getElapsed() * 1e3)
        / timer.getCount());
    for(size_t i = 0; i < Equator::ProcContext::N_ProcTimer; ++i)
    {
        const double elapsed = context.timers[i].getElapsed() * 1e3;
        const unsigned long long count = context.timers[i].getCount();
        LOG_DEBUG_STR("TIMER ms " << Equator::ProcContext::timerNames[i]
            << " total " << elapsed << " count " << count << " avg "
            << elapsed / count);
    }
}

void Equator::makeGridMapping()
{
    // Compute a mapping from cells of the solution grid to cell intervals in
    // the evaluation grid.
    const pair<Interval, vector<Interval> > mapFreq =
        makeAxisMapping(itsSolGrid[FREQ], itsEvalGrid[FREQ]);

    const pair<Interval, vector<Interval> > mapTime =
        makeAxisMapping(itsSolGrid[TIME], itsEvalGrid[TIME]);

    // Store the mapping for each axis.
    itsFreqIntervals = mapFreq.second;
    itsTimeIntervals = mapTime.second;

    itsIntersectionEmpty = itsFreqIntervals.empty() || itsTimeIntervals.empty();
    if(itsIntersectionEmpty)
    {
        LOG_WARN_STR("Intersection of the solution grid and the evaluation grid"
            " is empty.");
    }

    // Store the indices of the first and last cell of the solution grid that
    // intersect the evaluation grid.
    itsEvalStart = Location(mapFreq.first.start, mapTime.first.start);
    itsEvalEnd = Location(mapFreq.first.end, mapTime.first.end);
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
    ParmGroup solvablesLHS = itsLHS->solvables();
    ParmGroup solvablesRHS = itsRHS->solvables();

    ParmGroup solvables;
    std::set_union(solvablesLHS.begin(), solvablesLHS.end(),
        solvablesRHS.begin(), solvablesRHS.end(),
        std::inserter(solvables, solvables.begin()));

    for(ParmGroup::const_iterator solIt = solvables.begin(),
        solItEnd = solvables.end(); solIt != solItEnd; ++solIt)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*solIt);

        CoeffIndex::const_iterator indexIt = index.find(parm->getName());
        ASSERT(indexIt != index.end());

        const CoeffInterval &interval = indexIt->second;
        for(unsigned int i = 0; i < interval.length; ++i)
        {
            itsCoeffMap[PValueKey(parm->getId(), i)] = interval.start + i;
        }
    }
}

void Equator::makeExprCoeffMapping(const ValueSet &lhs, const ValueSet &rhs,
    ProcContext &context)
{
    ValueSet::const_iterator itLHS = lhs.begin(), endLHS = lhs.end();
    ValueSet::const_iterator itRHS = rhs.begin(), endRHS = rhs.end();

    unsigned int nCoeff = 0;
    while(itLHS != endLHS && itRHS != endRHS)
    {
        if(itLHS->first < itRHS->first)
        {
            // Look-up the coefficient index for this coefficient.
            context.index[nCoeff] = itsCoeffMap[itLHS->first];
            // Compute the partial derivative with respect to this coefficient.
            context.partial[nCoeff] = -itLHS->second;

            ++itLHS;
        }
        else if(itRHS->first < itLHS->first)
        {
            // Look-up the coefficient index for this coefficient.
            context.index[nCoeff] = itsCoeffMap[itRHS->first];
            // Compute the partial derivative with respect to this coefficient.
            context.partial[nCoeff] = itRHS->second;

            ++itRHS;
        }
        else
        {
            // Look-up coefficient index for this coefficient.
            context.index[nCoeff] = itsCoeffMap[itLHS->first];
            // Compute the partial derivative with respect to this coefficient.
            context.partial[nCoeff] = itRHS->second - itLHS->second;

            ++itLHS;
            ++itRHS;
        }

        ++nCoeff;
    }

    // Process any remaining coefficients on the left hand side.
    while(itLHS != endLHS)
    {
        // Look-up coefficient index for this coefficient.
        context.index[nCoeff] = itsCoeffMap[itLHS->first];
        // Compute the partial derivative with respect to this coefficient.
        context.partial[nCoeff] = -itLHS->second;

        ++itLHS;
        ++nCoeff;
    }

    // Process any remaining coefficients on the right hand side.
    while(itRHS != endRHS)
    {
        // Look-up coefficient index for this coefficient.
        context.index[nCoeff] = itsCoeffMap[itRHS->first];
        // Compute the partial derivative with respect to this coefficient.
        context.partial[nCoeff] = itRHS->second;

        ++itRHS;
        ++nCoeff;
    }

    // Store the number of coefficients for this expression.
    context.nCoeff = nCoeff;
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
// - Equator::ProcContext implementation                                      - //
// -------------------------------------------------------------------------- //
Equator::ProcContext::ProcContext()
    :   count(0)
{
}

void Equator::ProcContext::resize(unsigned int nCoeff)
{
    index.resize(nCoeff);
    partial.resize(nCoeff);
    partialRe.resize(nCoeff);
    partialIm.resize(nCoeff);
}

void Equator::ProcContext::reset()
{
    count = 0;
    for(size_t i = 0; i < Equator::ProcContext::N_ProcTimer; ++i)
    {
        timers[i].reset();
    }
}

string Equator::ProcContext::timerNames[Equator::ProcContext::N_ProcTimer] =
    {"EVAL_LHS",
    "EVAL_RHS",
    "MERGE_FLAGS",
    "EQUATE",
    "MAKE_COEFF_MAP",
    "TRANSPOSE",
    "MAKE_NORM"};

} //# namespace BBS
} //# namespace LOFAR
