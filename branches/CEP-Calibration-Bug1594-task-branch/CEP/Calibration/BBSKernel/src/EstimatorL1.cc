//# EstimatorL1.cc: Parameter estimation using the Levenberg-Marquardt
//# algorithm with a L1-norm weighting scheme.
//#
//# Copyright (C) 2010
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
#include <BBSKernel/EstimatorL1.h>

namespace LOFAR
{
namespace BBS
{

EstimatorL1::EstimatorL1(const VisBuffer::Ptr &lhs,
    const MeasurementExpr::Ptr &rhs)
    :   itsLHS(lhs),
        itsRHS(rhs),
        itsIntersectionEmpty(true),
        itsSelectedCellCount(0),
        itsCellChunkSize(1),
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

    itsEpsilon.reserve(3);
    itsEpsilon.push_back(1e-4);
    itsEpsilon.push_back(1e-5);
    itsEpsilon.push_back(1e-6);
}

EstimatorL1::~EstimatorL1()
{
    itsRHS->clearSolvables();
}

void EstimatorL1::setBaselineMask(const BaselineMask &mask)
{
    itsBlMap.clear();
    makeIndexMap(itsLHS->baselines(), itsRHS->baselines(), mask,
        back_inserter(itsBlMap));

    LOG_DEBUG_STR("Selected " << itsBlMap.size() << "/" << itsLHS->nBaselines()
        << " baseline(s) for processing");
}

void EstimatorL1::setCorrelationMask(const CorrelationMask &mask)
{
    itsCrMap.clear();
    makeIndexMap(itsLHS->correlations(), itsRHS->correlations(), mask,
        back_inserter(itsCrMap));

    LOG_DEBUG_STR("Selected " << itsCrMap.size() << "/"
        << itsLHS->nCorrelations() << " correlation(s) for processing");
}

void EstimatorL1::setOptions(const SolverOptions &options)
{
    itsOptions = options;
}

void EstimatorL1::setSolvables(const vector<string> &include,
    const vector<string> &exclude)
{
    itsRHS->setSolvables(ParmManager::instance().makeSubset(include,
        exclude, itsRHS->parms()));

    itsSolvables = itsRHS->solvables();
    if(itsSolvables.empty()) {
      LOG_WARN_STR("No parameters found matching the specified inclusion and"
        " exclusion criteria.");
    }

    // Regenerate coefficient map.
    makeCoeffMap();

    // Pre-allocate buffers for processing.
    itsProcContext.resize(itsCoeffCount);

    LOG_DEBUG_STR("#solvables: " << itsCoeffCount);
}

void EstimatorL1::setSolutionGrid(const Grid &grid)
{
    itsSolGrid = grid;

    // Regenerate solution cell map.
    makeCellMap();

    // By default select all the cells in the solution grid that intersect the
    // evaluation grid for processing.
    setCellSelection(itsEvalStart, itsEvalEnd);
}

void EstimatorL1::setCellSelection(const Location &start,
     const Location &end)
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

//        LOG_DEBUG_STR("Solution cells to process (solution grid coordinates):"
//            " [(" << itsSelectionStart.first << "," << itsSelectionStart.second
//            << "),(" << itsSelectionEnd.first << "," << itsSelectionEnd.second
//            << ")]");

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

//        LOG_DEBUG_STR("Samples to process (observation grid coordinates): [("
//            << itsReqStart.first << "," << itsReqStart.second << "),("
//            << itsReqEnd.first << "," << itsReqEnd.second << ")]");

        itsRHS->setEvalGrid(itsLHS->grid().subset(itsReqStart, itsReqEnd));
    }
}

void EstimatorL1::setCellChunkSize(size_t size)
{
    itsCellChunkSize =
        std::min(size, itsEvalEnd.second - itsEvalStart.second + 1);
}

bool EstimatorL1::isSelectionEmpty() const
{
    return itsIntersectionEmpty || itsBlMap.empty() || itsCrMap.empty();
}

void EstimatorL1::process()
{
    // Assign solution grid to solvables. This is done here instead of in
    // setSolutionGrid because the solution grid can only be set once for any
    // given parameter (after setting it once, it can only be set to exactly
    // the same grid).
    ParmManager::instance().setGrid(itsSolGrid, itsSolvables);

    // Check if there is observed and model data available for the current
    // solution grid.
    if(isSelectionEmpty())
    {
        LOG_WARN_STR("Cannot estimate parameters because there is no observed"
            " data and/or model data available for the active solution grid.");
        return;
    }

    // Check if there are coefficients to estimate.
    if(itsCoeffCount == 0)
    {
        LOG_WARN_STR("Cannot estimate parameters because the current parameter"
            " selection does not match any of the model parameters.");
        return;
    }

    itsProcTimer.start();

    // Compute the number of cell chunks to process.
    size_t nCellChunks =
        static_cast<size_t>(ceil(static_cast<double>(itsSolGrid[TIME]->size())
            / itsCellChunkSize));

    // Loop over all cell chunks.
    Location chunkStart(0, 0);
    Location chunkEnd(itsSolGrid[FREQ]->size() - 1,
        itsSolGrid[TIME]->size() - 1);
    for(size_t cellChunk = 0; cellChunk < nCellChunks; ++cellChunk)
    {
        // Compute end cell of current cell chunk.
        chunkEnd.second = std::min(chunkStart.second + itsCellChunkSize - 1,
            itsSolGrid[TIME]->size() - 1);

        // Notify model of the changed evaluation domain.
        setCellSelection(chunkStart, chunkEnd);

        // Check if there is observed and model data available for the current
        // cell selection.
        if(itsSelectedCellCount == 0)
        {
            LOG_WARN_STR("chunk: " << cellChunk << "/" << nCellChunks
                << " iterations: 0 cells: 0 status: skipped");

            // Move to the next cell chunk.
            chunkStart.second += itsCellChunkSize;

            continue;
        }

        // ...
        initSolutionCells(itsSelectionStart, itsSelectionEnd);

        bool done = false;
        size_t nIterations = 0, nConverged = 0, nSingular = 0, nNoReduction = 0;
        while(!done)
        {
            // Perform a single LSQ iteration.
            iterate();

            // Update solution cell state.
            done = true;
            nConverged = nNoReduction = nSingular = 0;
            for(vector<Cell>::iterator it = itsCells.begin(),
                end = itsCells.end(); it != end; ++it)
            {
                // Handle LSQFit status codes.
                switch(it->solver.isReady())
                {
                    case casa::LSQFit::NONREADY:
                        done = false;
                        break;

                    case casa::LSQFit::SOLINCREMENT:
                    case casa::LSQFit::DERIVLEVEL:
                        ++nConverged;
                        break;

                    case casa::LSQFit::NOREDUCTION:
                        ++nNoReduction;
                        break;

                    case casa::LSQFit::SINGULAR:
                        ++nSingular;
                        break;

                    default:
                        break;
                }

                if(it->solver.isReady() && it->index < itsEpsilon.size())
                {
                    ++it->index;
                    if(it->index < itsEpsilon.size())
                    {
//                        it->coeffTmp = it->coeff;

//                        if(it->index > 1)
//                        {
//                            // Predict starting point.
//                            double d1 = std::sqrt(itsEpsilon[it->index - 1]) - std::sqrt(itsEpsilon[it->index - 2]);
//                            double d2 = std::sqrt(itsEpsilon[it->index]) - std::sqrt(itsEpsilon[it->index - 1]);
//                            for(size_t i = 0; i < itsCoeffCount; ++i)
//                            {
//                                it->coeff[i] += (it->coeff[i] - it->coeffPrev[i]) / (d1*d2);
//                            }
//                        }

//                        // Store a copy of the solutions for this epsilon.
//                        it->coeffPrev = it->coeffTmp;

                        // Move to next epsilon.
                        initSolver(*it);
                        it->epsilon = itsEpsilon[it->index];
                        done = false;
                    }
                }
            }

//            LOG_DEBUG_STR("chunk: " << cellChunk << "/" << nCellChunks
//                << " iteration: " << nIterations << " cells: "
//                << itsCells.size() << " status: " << nConverged << "/"
//                << nFailed << "/" << itsCells.size() - nConverged - nFailed
//                << " converged/failed/stopped.");

            // Update solvables.
            for(vector<Cell>::iterator it = itsCells.begin(),
                end = itsCells.end(); it != end; ++it)
            {
                storeUnknowns(*it);
            }

            // Notify model.
            itsRHS->solvablesChanged();

            ++nIterations;
        }

        LOG_DEBUG_STR("chunk: " << cellChunk << "/" << nCellChunks
            << " iterations: " << nIterations << " cells: " << itsCells.size()
            << " status: " << nConverged << "/" << nNoReduction << "/"
            << nSingular << "/"
            << itsCells.size() - nConverged - nNoReduction - nSingular
            << " converged/noreduction/singular/stopped.");

//        (propagte solutions to next chunk)

        // Move to the next cell chunk.
        chunkStart.second += itsCellChunkSize;
    }

    itsProcTimer.stop();
}

void EstimatorL1::iterate()
{
    FRange freqFRange(itsReqStart.first, itsReqEnd.first + 1);
    FRange timeFRange(itsReqStart.second, itsReqEnd.second + 1);
    FSlice flag(itsLHS->flags[boost::indices[FRange()][timeFRange]
        [freqFRange][FRange()]]);

    WRange freqWRange(itsReqStart.first, itsReqEnd.first + 1);
    WRange timeWRange(itsReqStart.second, itsReqEnd.second + 1);
    WSlice weight(itsLHS->weights[boost::indices[WRange()][timeWRange]
        [freqWRange][WRange()]]);

    SRange freqSRange(itsReqStart.first, itsReqEnd.first + 1);
    SRange timeSRange(itsReqStart.second, itsReqEnd.second + 1);
    SSlice sample(itsLHS->samples[boost::indices[SRange()][timeSRange]
        [freqSRange][SRange()]]);

    // Construct equations for all baselines.
    for(size_t i = 0; i < itsBlMap.size(); ++i)
    {
        procExpr(itsProcContext, flag, weight, sample, itsBlMap[i]);
    }

//    // Get some statistics from the solver. Note that the chi squared is
//    // valid for the _previous_ iteration. The solver cannot compute the
//    // chi squared directly after an iteration, because it needs the new
//    // condition equations for that and these are computed by the kernel.
//    casa::uInt rank, nun, np, ncon, ner, *piv;
//    casa::Double *nEq, *known, *constr, *er, *sEq, *sol, prec, nonlin;
//    cell.solver.debugIt(nun, np, ncon, ner, rank, nEq, known, constr, er,
//        piv, sEq, sol, prec, nonlin);
//    ASSERT(er && ner > Solver::SUMLL);

//    double lmFactor = nonlin;
//    double chiSqr = er[Solver::SUMLL] / std::max(er[Solver::NC] + nun, 1.0);

    for(vector<Cell>::iterator it = itsCells.begin(), end = itsCells.end();
        it != end; ++it)
    {
        // Perform an iteration.
        casa::uInt rank;
        it->solver.solveLoop(rank, &(it->coeff[0]), itsOptions.useSVD);
    }

//    // Record solution and statistics.
//    CellSolution solution(static_cast<uint32>(cellId));
//    solution.coeff = cell.coeff;
//    solution.ready = (cell.solver.isReady() != Solver::NONREADY);
//    solution.resultText = cell.solver.readyText();
//    solution.rank = rank;
//    solution.chiSqr = chiSqr;
//    solution.lmFactor = lmFactor;
}

void EstimatorL1::procExpr(ProcContext &context,
    const EstimatorL1::FSlice &flagLHS,
    const EstimatorL1::WSlice &weightLHS,
    const EstimatorL1::SSlice &valueLHS,
    const pair<size_t, size_t> &idx)
{
    // Evaluate the right hand side.
    context.timers[ProcContext::EVAL_EXPR].start();
    const JonesMatrix RHS = itsRHS->evaluate(idx.second);
    context.timers[ProcContext::EVAL_EXPR].stop();

    // If the model contains no flags, assume no samples are flagged.
    // TODO: This incurs a cost for models that do not contain flags because
    // a call to virtual FlagArray::operator() is made for each sample.
    FlagArray flagRHS(flag_t(0));
    if(RHS.hasFlags())
    {
        flagRHS = RHS.flags();
    }

    // If all model visibilities are flagged, skip this baseline.
    if(flagRHS.rank() == 0 && flagRHS(0, 0) != 0)
    {
        return;
    }

    context.timers[ProcContext::EQUATE].start();

    const size_t nTime = valueLHS.shape()[1];
    const size_t nFreq = valueLHS.shape()[2];

    for(size_t cr = 0; cr < itsCrMap.size(); ++cr)
    {
        const size_t crLHS = itsCrMap[cr].first;
        const size_t crRHS = itsCrMap[cr].second;

        const Element elementRHS = RHS.element(crRHS);

        // If there are no coefficients to fit, continue to the next
        // correlation.
        if(elementRHS.size() == 1)
        {
            continue;
        }

        // Determine a mapping from sequential coefficient number to coefficient
        // index in the condition equations.
        context.timers[ProcContext::MAKE_COEFF_MAP].start();
        makeElementCoeffMap(elementRHS, context);
        context.timers[ProcContext::MAKE_COEFF_MAP].stop();

        Matrix valueRHS = elementRHS.value();

        for(size_t t = 0; t < nTime; ++t)
        {
            const size_t eqIdx = (itsTimeMap[itsEvalReqStart.second + t]
                - itsEvalSelStart.second) * (itsEvalSelEnd.first
                - itsEvalSelStart.first + 1);

            for(size_t f = 0; f < nFreq; ++f)
            {
                if(flagLHS[idx.first][t][f][crLHS] || flagRHS(f, t))
                {
                    continue;
                }

                const size_t sampleEqIdx =
                    eqIdx + (itsFreqMap[itsEvalReqStart.first + f]
                        - itsEvalSelStart.first);
                ASSERT(sampleEqIdx < itsCells.size());
                Cell &cell = itsCells[sampleEqIdx];

                // Update statistics.
                ++context.count;

                // Compute right hand side of the equation pair.
                const dcomplex residual = valueLHS[idx.first][t][f][crLHS]
                    - valueRHS.getDComplex(f, t);

                // Tranpose the partial derivatives.
                context.timers[ProcContext::TRANSPOSE].start();
                for(size_t i = 0; i < context.nCoeff; ++i)
                {
                    const dcomplex partial =
                        context.partial[i].getDComplex(f, t);

                    context.partialRe[i] = real(partial);
                    context.partialIm[i] = imag(partial);
                }
                context.timers[ProcContext::TRANSPOSE].stop();

                // Compute L1 weights.
                double weight = weightLHS[idx.first][t][f][crLHS]
                    / std::sqrt(std::abs(residual) + cell.epsilon);

                // Generate condition equations.
                context.timers[ProcContext::MAKE_NORM].start();
                cell.solver.makeNorm(context.nCoeff,
                    &(context.index[0]),
                    &(context.partialRe[0]),
                    weight,
                    real(residual));

                cell.solver.makeNorm(context.nCoeff,
                    &(context.index[0]),
                    &(context.partialIm[0]),
                    weight,
                    imag(residual));
                context.timers[ProcContext::MAKE_NORM].stop();
            }
        }
    }

    context.timers[ProcContext::EQUATE].stop();
}

void EstimatorL1::initSolutionCells(const Location &start,
    const Location &end)
{
    const size_t nCells = (end.first - start.first + 1) * (end.second
        - start.second + 1);

    itsCells.clear();
    itsCells.reserve(nCells);

    CellIterator it(start, end);
    for(; !it.atEnd(); ++it)
    {
        Cell cell;
        cell.location = *it;
        cell.epsilon = itsEpsilon[0];
        cell.index = 0;

        initSolver(cell);

        loadUnknowns(cell);

        itsCells.push_back(cell);
    }
}

void EstimatorL1::initSolver(Cell &cell)
{
    cell.solver = casa::LSQFit(static_cast<casa::uInt>(itsCoeffCount));
    cell.solver.setMaxIter(itsOptions.maxIter);
    cell.solver.setEpsValue(itsOptions.epsValue);
    cell.solver.setEpsDerivative(itsOptions.epsDerivative);
    cell.solver.set(itsOptions.colFactor, itsOptions.lmFactor);
    cell.solver.setBalanced(itsOptions.balancedEq);
}

void EstimatorL1::storeUnknowns(const Cell &cell) const
{
    size_t i = 0;

    for(ParmGroup::const_iterator it = itsSolvables.begin(),
        end = itsSolvables.end(); it != end; ++it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*it);
        ASSERT(i + parm->getCoeffCount() <= cell.coeff.size());

        parm->setCoeff(cell.location, &(cell.coeff[i]), parm->getCoeffCount());
        i += parm->getCoeffCount();
    }
}

void EstimatorL1::loadUnknowns(Cell &cell) const
{
    cell.coeff.clear();

    for(ParmGroup::const_iterator it = itsSolvables.begin(),
        end = itsSolvables.end(); it != end; ++it)
    {
        ParmProxy::Ptr parm = ParmManager::instance().get(*it);

        vector<double> tmp = parm->getCoeff(cell.location);
        cell.coeff.insert(cell.coeff.end(), tmp.begin(), tmp.end());
    }
}

Interval<size_t>
EstimatorL1::findContainedCellRange(const Axis::ShPtr &axis,
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

void EstimatorL1::makeEvalGrid()
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

void EstimatorL1::makeCellMap()
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
}

void EstimatorL1::makeCoeffMap()
{
    // Clear information that needs to be regenerated.
    itsCoeffCount = 0;
    itsCoeffMap.clear();

    size_t index = 0;
    for(ParmGroup::const_iterator sol_it = itsSolvables.begin(),
        sol_end = itsSolvables.end(); sol_it != sol_end; ++sol_it)
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

void EstimatorL1::makeElementCoeffMap(const Element &element,
    ProcContext &context)
{
    size_t nCoeff = 0;
    for(Element::const_iterator it = element.begin(), end = element.end();
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

void EstimatorL1::clearStats()
{
    itsProcTimer.reset();
    itsProcContext.clearStats();
}

void EstimatorL1::dumpStats(ostream &out) const
{
    const ProcContext &context = itsProcContext;

    double elapsed = itsProcTimer.getElapsed();
    const double speed = elapsed > 0.0 ? context.count / elapsed : 0.0;
    unsigned long long count = itsProcTimer.getCount();
    double average = count > 0 ? elapsed / count : 0.0;

    out << "EstimatorL1 statistics:" << endl;
    out << "Speed: " << fixed << speed << " samples/s" << endl;
    out << "No. of samples processed (unflagged): " << fixed << context.count
        << endl;
    out << "TIMER s ESTIMATORLM ALL total " << elapsed << " count " << count
        << " avg " << average << endl;

    for(size_t i = 0; i < EstimatorL1::ProcContext::N_ProcTimer; ++i)
    {
        elapsed = context.timers[i].getElapsed();
        count = context.timers[i].getCount();
        average = count > 0 ? elapsed / count : 0.0;

        out << "TIMER s ESTIMATORLM " << EstimatorL1::ProcContext::timerNames[i]
            << " total" << " " << elapsed << " count " << count << " avg "
            << elapsed / count << endl;
    }
}

// -------------------------------------------------------------------------- //
// - EstimatorL1::ProcContext implementation                                - //
// -------------------------------------------------------------------------- //
EstimatorL1::ProcContext::ProcContext()
    :   count(0)
{
}

void EstimatorL1::ProcContext::resize(size_t nCoeff)
{
    index.resize(nCoeff);
    partial.resize(nCoeff);
    partialRe.resize(nCoeff);
    partialIm.resize(nCoeff);
}

void EstimatorL1::ProcContext::clearStats()
{
    count = 0;
    for(size_t i = 0; i < EstimatorL1::ProcContext::N_ProcTimer; ++i)
    {
        timers[i].reset();
    }
}

string
EstimatorL1::ProcContext::timerNames[EstimatorL1::ProcContext::N_ProcTimer] =
    {"EVAL_EXPR",
    "EQUATE",
    "MAKE_COEFF_MAP",
    "TRANSPOSE",
    "MAKE_NORM"};

} //# namespace BBS
} //# namespace LOFAR
