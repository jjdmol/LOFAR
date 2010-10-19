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

#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

Equator::Equator(const VisData::Ptr &chunk, const Model::Ptr &model,
        const CoeffIndex &index, const Grid &grid, uint nMaxCells,
        uint nThreads)
    :   itsChunk(chunk),
        itsModel(model),
        itsSolGrid(grid),
        itsMaxCellCount(nMaxCells),
        itsThreadCount(nThreads)
{
    ASSERT(itsChunk);
    ASSERT(itsModel);

#ifndef _OPENMP
    // Ignore thread count specified in constructor.
    itsThreadCount = 1;
#endif

    // By default, select all the baselines and polarizations products
    // available.
    const VisDimensions &dims = itsChunk->getDimensions();
    setSelection(dims.getBaselines(), dims.getPolarizations());

    // Create a mapping for each axis that maps from cells in the solution grid
    // to cell intervals in the observation (chunk) grid.
    makeGridMapping();

    // Create a mapping that maps each (parmId, coeffId) combination to an
    // index.
    makeCoeffMapping(index);

    // Pre-allocate thread private casa::LSQFit instances.
    makeContexts();
}

Equator::~Equator()
{
}

void Equator::setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products)
{
    itsBaselines = baselines;

    // Determine product mask.
    const VisDimensions &dims = itsChunk->getDimensions();

    fill(&(itsProductMask[0]), &(itsProductMask[4]), -1);

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

void Equator::process(vector<CellEquation> &result, const Location &start,
    const Location &end)
{
    // Check if [start, end] is a valid range.
    ASSERTSTR(start.first <= end.first && start.second <= end.second,
        "Invalid cell range specified.");
    // Check number of cells in the request.
    ASSERTSTR((end.first - start.first + 1) * (end.second - start.second + 1)
        <= itsMaxCellCount, "Number of cells to process too large.");
    // Check if end location points to a valid cell in the solution grid.
    ASSERTSTR(end.first < itsSolGrid[FREQ]->size()
        && end.second < itsSolGrid[TIME]->size(),
        "Cell range extends outside of solution grid.");

    // If no observed visibilities are available for the requested cell range,
    // then exit.
    if(end.first < itsStartCell.first
        || start.first > itsEndCell.first
        || end.second < itsStartCell.second
        || start.second > itsEndCell.second)
    {
        result.clear();
        return;
    }

    resetTimers();
    itsTimers[PROCESS].start();

    LOG_DEBUG_STR("Cells requested (solution grid relative): [("
        << start.first << "," << start.second << "),(" << end.first << ","
        << end.second << ")]");

    // Clip requested cell range against available visibility data.
    Location reqStart(std::max(start.first, itsStartCell.first),
        std::max(start.second, itsStartCell.second));
    Location reqEnd(std::min(end.first, itsEndCell.first),
         std::min(end.second, itsEndCell.second));
    const size_t nCells =
        (end.first - start.first + 1) * (end.second - start.second + 1);

    LOG_DEBUG_STR("Cells to process (solution grid relative): [("
        << reqStart.first << "," << reqStart.second << "),(" << reqEnd.first
        << "," << reqEnd.second << ")]");

    // Initialize the result and the thread context of the main thread.
    result.resize(nCells);

    size_t index = 0;
    CellIterator cellIt(reqStart, reqEnd);
    while(!cellIt.atEnd())
    {
        // Set cell id (relative to the solution grid).
        result[index].id = itsSolGrid.getCellId(*cellIt);
        result[index].equation.set(static_cast<uint>(itsCoeffMap.size()));
        itsContexts[0].eq[index] = &(result[index].equation);
        ++index;
        ++cellIt;
    }

    // Transform to chunk relative coordinates.
    reqStart.first -= itsStartCell.first;
    reqStart.second -= itsStartCell.second;
    reqEnd.first -= itsStartCell.first;
    reqEnd.second -= itsStartCell.second;

    LOG_DEBUG_STR("Cells to process (chunk relative): [(" << reqStart.first
        << "," << reqStart.second << "),(" << reqEnd.first << ","
        << reqEnd.second << ")]");

    // Get frequency / time grid information.
    const Location visStart(itsFreqIntervals[reqStart.first].start,
        itsTimeIntervals[reqStart.second].start);
    const Location visEnd(itsFreqIntervals[reqEnd.first].end,
        itsTimeIntervals[reqEnd.second].end);

    LOG_DEBUG_STR("Visibilities to process (chunk relative): [("
        << visStart.first << "," << visStart.second << "),(" << visEnd.first
        << "," << visEnd.second << ")]");

    // Create request grid.
    const Grid &visGrid = itsChunk->getDimensions().getGrid();
    itsModel->setRequestGrid(visGrid.subset(visStart, visEnd));

//    LOG_DEBUG_STR("Request dimensions: " << itsBaselines.size() << "b x "
//        << request.getTimeslotCount() << "t x " << request.getChannelCount()
//        << "c x " << 4 - count(&(itsProductMask[0]), &(itsProductMask[4]), -1)
//        << "p");

    // Precompute.
//    LOG_DEBUG_STR("Precomputing...");
//    itsTimers[PRECOMPUTE].start();
//    itsModel->precalculate(request);
//    itsTimers[PRECOMPUTE].stop();
//    LOG_DEBUG_STR("Precomputing... done");

#ifdef _OPENMP
    // Reset casa::LSQFit instances for each non-main thread.
    for(size_t i = 1; i < itsThreadCount; ++i)
    {
        for(size_t j = 0; j < itsMaxCellCount; ++j)
        {
            itsThreadEq[i - 1][j].reset();
        }
    }
#endif

    itsTimers[COMPUTE].start();
#pragma omp parallel for num_threads(itsThreadCount) schedule(dynamic)
    for(int i = 0; i < static_cast<int>(itsBaselines.size()); ++i)
    {
#ifdef _OPENMP
        blConstruct(omp_get_thread_num(), itsBaselines[i], reqStart, reqEnd,
            visStart);
#else
        blConstruct(0, itsBaselines[i], reqStart, reqEnd, visStart);
#endif
    }
    itsTimers[COMPUTE].stop();

#ifdef _OPENMP
    if(itsContexts.size() > 1)
    {
        // Merge thread private equations into a single result.
        // TODO: Re-implement the following in O(log(N)) steps.
        itsTimers[MERGE].start();

        ThreadContext &main = itsContexts[0];

      	for(size_t i = 1; i < itsContexts.size(); ++i)
    	  {
            ThreadContext &thread = itsContexts[i];

            for(size_t j = 0; j < nCells; ++j)
            {
                main.eq[j]->merge(*thread.eq[j]);
            }
        }

        itsTimers[MERGE].stop();
    }
#endif

    // Reset thread context of main thread.
    fill(itsContexts[0].eq.begin(), itsContexts[0].eq.end(),
        static_cast<casa::LSQFit*>(0));

    itsTimers[PROCESS].stop();
    printTimers();
}

void Equator::blConstruct(uint threadId, const baseline_t &baseline,
    const Location &cellStart, const Location &cellEnd,
    const Location &visStart)
{
    ThreadContext &context = itsContexts[threadId];

    // Find baseline index.
    // NB. VisDimensions::getBaselineIndex() could throw an exception.
    const VisDimensions &dims = itsChunk->getDimensions();
    const uint blIndex = dims.getBaselineIndex(baseline);

    // Evaluate the model.
    context.timers[ThreadContext::MODEL_EVAL].start();
    const JonesMatrix result = itsModel->evaluate(baseline);

    // If the result contains no flags, assume no sample is flagged.
    // TODO: This incurs a cost for results that do not contain flags because
    // a call to virtual FlagArray::operator() is made for each sample.
    const FlagArray flags =
        (result.hasFlags() ? result.flags() : FlagArray((FlagType())));
    context.timers[ThreadContext::MODEL_EVAL].stop();

    // Construct equations.
    context.timers[ThreadContext::PROCESS].start();
    for(unsigned int prod = 0; prod < 4; ++prod)
    {
        if(itsProductMask[prod] == -1)
        {
            continue;
        }

        const int extProd = itsProductMask[prod];

//        typedef boost::multi_array<sample_t, 4>::index_range Range;
//        typedef boost::multi_array<sample_t, 4>::array_view<2>::type View;
//        View vdata(itsChunk->vis_data[boost::indices[blIndex][Range()][Range()]
//            [extProd]]);

        // Determine which parameters have partial derivatives (e.g. when
        // solving for station-bound parameters, only a few parameters
        // per baseline are relevant).
        context.timers[ThreadContext::BUILD_INDEX].start();
        unsigned int nSolvables = 0;

        ValueSet::const_iterator it = result.getValueSet(prod).begin();
        ValueSet::const_iterator itEnd = result.getValueSet(prod).end();
        while(it != itEnd)
        {
            // Get a reference to the data.
            context.partial[nSolvables] = it->second;

            // Look-up coefficient index for the perturbed coefficient.
            context.index[nSolvables] = itsCoeffMap[it->first];

            ++nSolvables;
            ++it;
        }
        context.timers[ThreadContext::BUILD_INDEX].stop();

        // If no partial derivatives were found, continue.
        if(nSolvables == 0)
        {
            continue;
        }

        unsigned int eqIndex = 0;
        const Matrix model = result.getValueSet(prod).value();

        // Loop over all selected cells and generate equations, adding them
        // to the associated solver.
        CellIterator cellIt(cellStart, cellEnd);
        while(!cellIt.atEnd())
        {
            // Samples to process (observed visibilities).
            const Interval &chInterval = itsFreqIntervals[cellIt->first];
            const Interval &tsInterval = itsTimeIntervals[cellIt->second];

//            cout << "ch: " << chInterval.start << "-" << chInterval.end
//                << " ts: " << tsInterval.start << "-" << tsInterval.end << endl;

//            size_t visOffset = (tsInterval.start - visStart.second)
//                * nChannels + (chInterval.start - visStart.first);

            casa::LSQFit *eq = context.eq[eqIndex];

            for(size_t ts = tsInterval.start; ts <= tsInterval.end; ++ts)
            {
                // Skip timeslot if flagged.
                if(itsChunk->tslot_flag[blIndex][ts])
                {
//                    visOffset += nChannels;
                    continue;
                }

                // Construct two equations for each unflagged visibility.
                for(size_t ch = chInterval.start; ch <= chInterval.end; ++ch)
                {
                    if(!itsChunk->vis_flag[blIndex][ts][ch][extProd]
                        && !flags((int)(ch - visStart.first),
                            (int)(ts - visStart.second)))
                    {
                        // Update statistics.
                        ++context.count;

                        // Compute right hand side of the equation pair.
                        const complex_t rhs =
                            static_cast<complex_t>
                                (itsChunk->vis_data[blIndex][ts][ch][extProd])
                            - model.getDComplex((int)(ch - visStart.first),
                                (int)(ts - visStart.second));

                        // Approximate partial derivatives (forward differences)
                        // TODO: Remove this transpose.
                        context.timers[ThreadContext::TRANSPOSE].start();
                        for(size_t i = 0; i < nSolvables; ++i)
                        {
                            const complex_t partial =
                                context.partial[i].getDComplex
                                    ((int)(ch - visStart.first),
                                    (int)(ts - visStart.second));

                            context.partialRe[i] = real(partial);
                            context.partialIm[i] = imag(partial);
                        }
                        context.timers[ThreadContext::TRANSPOSE].stop();

                        context.timers[ThreadContext::MAKE_NORM].start();
                        eq->makeNorm(nSolvables,
                            &(context.index[0]),
                            &(context.partialRe[0]),
                            1.0,
                            real(rhs));

                        eq->makeNorm(nSolvables,
                            &(context.index[0]),
                            &(context.partialIm[0]),
                            1.0,
                            imag(rhs));
                        context.timers[ThreadContext::MAKE_NORM].stop();
                    } // !itsChunk->vis_flag[blIndex][ts][ch][extProd]
//                    // Move to next channel.
//                    ++visOffset;
                } // for(size_t ch = chStart; ch < chEnd; ++ch)

                // Move to next timeslot.
//                visOffset +=
//                    nChannels - (chInterval.end - chInterval.start + 1);
            } // for(size_t ts = tsStart; ts < tsEnd; ++ts)

            // Move to next cell.
            ++eqIndex;
            ++cellIt;
        }
    }

    context.timers[ThreadContext::PROCESS].stop();
}

void Equator::makeContexts()
{
    ASSERTSTR(itsCoeffMap.size() > 0, "Call Equator::makeCoeffIndex() first.");

    // Pre-allocate thread private casa::LSQFit instances and initialize an
    // array of pointers to it for thread 1 and upwards.
    itsContexts.resize(itsThreadCount);
    for(size_t i = 0; i < itsThreadCount; ++i)
    {
        itsContexts[i].resize(itsCoeffMap.size(), itsMaxCellCount);
    }

#ifdef _OPENMP
    if(itsThreadCount > 1)
    {
        // Initialize thread-private data structures.
        itsThreadEq.resize(boost::extents[itsThreadCount - 1][itsMaxCellCount]);
        for(size_t i = 1; i < itsThreadCount; ++i)
        {
            for(size_t j = 0; j < itsMaxCellCount; ++j)
            {
                itsThreadEq[i - 1][j].set
                    (static_cast<uint>(itsCoeffMap.size()));
                itsContexts[i].eq[j] = &itsThreadEq[i - 1][j];
            }
        }
    }
#endif
}

void Equator::makeGridMapping()
{
    const Grid &visGrid = itsChunk->getDimensions().getGrid();

    // Compute overlap between the solution grid and the current chunk.
    Box overlap = itsSolGrid.getBoundingBox() & visGrid.getBoundingBox();
    ASSERTSTR(!overlap.empty(), "No overlap between the solution grid and the"
        " current chunk.");

    // Find the first and last cell that intersects the current chunk.
    itsStartCell = itsSolGrid.locate(overlap.lower());
    itsEndCell = itsSolGrid.locate(overlap.upper(), false);

    // The end cell is _inclusive_ by convention.
    const size_t nFreqCells = itsEndCell.first - itsStartCell.first + 1;
    const size_t nTimeCells = itsEndCell.second - itsStartCell.second + 1;

    // Map cells to inclusive sample intervals (frequency axis).
    Axis::ShPtr visAxis = visGrid[FREQ];
    Axis::ShPtr solAxis = itsSolGrid[FREQ];

    Interval interval;
    itsFreqIntervals.resize(nFreqCells);
    for(size_t i = 0; i < nFreqCells; ++i)
    {
        interval.start =
            visAxis->locate(std::max(solAxis->lower(itsStartCell.first + i),
                overlap.lowerX()));
        interval.end =
            visAxis->locate(std::min(solAxis->upper(itsStartCell.first + i),
                overlap.upperX()), false);
        itsFreqIntervals[i] = interval;
    }

    // Map cells to inclusive sample intervals (time axis).
    visAxis = visGrid[TIME];
    solAxis = itsSolGrid[TIME];

    itsTimeIntervals.resize(nTimeCells);
    for(size_t i = 0; i < nTimeCells; ++i)
    {
        interval.start =
            visAxis->locate(std::max(solAxis->lower(itsStartCell.second + i),
                overlap.lowerY()));
        interval.end =
            visAxis->locate(std::min(solAxis->upper(itsStartCell.second + i),
                overlap.upperY()), false);
        itsTimeIntervals[i] = interval;
    }
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
        for(uint i = 0; i < interval.length; ++i)
        {
            itsCoeffMap[PValueKey(parm->getId(), i)] = interval.start + i;
        }

        ++solIt;
    }
}

void Equator::resetTimers()
{
    for(size_t i = 0; i < N_Timer; ++i)
    {
        itsTimers[i].reset();
    }

    for(size_t i = 0; i < itsContexts.size(); ++i)
    {
        itsContexts[i].count = 0;

        for(size_t j = 0; j < ThreadContext::N_ThreadTimer; ++j)
        {
            itsContexts[i].timers[j].reset();
        }
    }
}

void Equator::printTimers()
{
    LOG_DEBUG("Timings:");

    unsigned long long count = 0;
    for(size_t i = 0; i < itsContexts.size(); ++i)
    {
        count += itsContexts[i].count;
    }
    LOG_DEBUG_STR("Processing speed: " << static_cast<size_t>((count
        / itsTimers[PROCESS].getElapsed())) << " vis/s");

    LOG_DEBUG_STR("Total #visibilities (unflagged): " << fixed << count);
    LOG_DEBUG_STR("Total time: " << itsTimers[PROCESS].getElapsed() << " s");
    LOG_DEBUG_STR("> Precomputation: " << itsTimers[PRECOMPUTE].getElapsed()
        * 1000.0 << " ms");
    LOG_DEBUG_STR("> Computation: " << itsTimers[COMPUTE].getElapsed()
        * 1000.0 << " ms");

#if defined(LOFAR_DEBUG) || defined(LOFAR_BBS_VERBOSE)
    for(size_t i = 0; i < ThreadContext::N_ThreadTimer; ++i)
    {
        unsigned long long count = 0;
        double sum = 0.0;

        // Merge thread private timers.
        for(size_t j = 0; j < itsContexts.size(); ++j)
        {
            // Convert from s to ms.
            sum += itsContexts[j].timers[i].getElapsed() * 1000.0;
            count += itsContexts[j].timers[i].getCount();
        }

        if(count == 0)
        {
            LOG_DEBUG_STR("> > " << ThreadContext::timerNames[i]
                << ": timer not used.");
        }
        else
        {
            LOG_DEBUG_STR("> > " << ThreadContext::timerNames[i] << ": total: "
                << sum << " ms, count: " << count << ", avg: " << sum / count
                << " ms");
        }
    }
#endif

    LOG_DEBUG_STR("> Merge equations: " << itsTimers[MERGE].getElapsed()
        * 1000.0 << " ms");
}

// -------------------------------------------------------------------------- //
// - ThreadContext implementation                                           - //
// -------------------------------------------------------------------------- //

string Equator::ThreadContext::timerNames[Equator::ThreadContext::N_ThreadTimer]
    = {"Model evaluation",
        "Process",
        "Build solvable index",
        "Transpose partial derivatives",
        "casa::LSQFit::makeNorm()"};

Equator::ThreadContext::ThreadContext()
    : count(0)
{
}

Equator::ThreadContext::~ThreadContext()
{
}

void Equator::ThreadContext::resize(uint nSolvables, uint nMaxCells)
{
    eq.resize(nMaxCells);
    index.resize(nSolvables);
    partial.resize(nSolvables);
    partialRe.resize(nSolvables);
    partialIm.resize(nSolvables);
}

void Equator::ThreadContext::clear(bool clearEq)
{
    if(clearEq)
    {
        eq.clear();
    }

    index.clear();
    partial.clear();
    partialRe.clear();
    partialIm.clear();
}

} //# namespace BBS
} //# namespace LOFAR
