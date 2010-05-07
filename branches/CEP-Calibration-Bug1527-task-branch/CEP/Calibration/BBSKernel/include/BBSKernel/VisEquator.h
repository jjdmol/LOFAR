//# VisEquator.h: Generate condition equations based on a buffer of observed and
//# a buffer of simulated visibilities.
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

#ifndef LOFAR_BBSKERNEL_VISEQUATOR_H
#define LOFAR_BBSKERNEL_VISEQUATOR_H

// \file
// Generate condition equations based on a buffer of observed and a buffer of
// simulated visibilities.

#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/CorrelationMask.h>
#include <BBSKernel/MeasurementExpr.h>
#include <BBSKernel/SolverInterfaceTypes.h>
#include <BBSKernel/Types.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/Expr/ExprValue.h>

#include <Common/Timer.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class VisEquator
{
public:
    typedef shared_ptr<VisEquator>          Ptr;
    typedef shared_ptr<const VisEquator>    ConstPtr;

    VisEquator(const VisData::Ptr &lhs, const MeasurementExpr::Ptr &rhs);
    ~VisEquator();

    // Set the solution grid.
    void setSolutionGrid(const Grid &grid);

    // Set the (inclusive) range of cells of the solution grid to process.
    void setCellSelection(const Location &start, const Location &end);

    // Returns the number of cells in the selection for which observed
    // visibilities are available.
    size_t nSelectedCells() const;

    // Restrict processing to the baselines included in the mask.
    void setBaselineMask(const BaselineMask &mask);

    // Restrict processing to the correlations included in the mask.
    void setCorrelationMask(const CorrelationMask &mask);

    // Is the current visibility selection empty? The visibility selection is
    // determined by:
    //     * The intersection between the model, measurement, and solution grid
    //       domains.
    //     * The selected baselines and correlations.
    bool isSelectionEmpty() const;

    // Get the set of parameters the measurement expression depends on.
    ParmGroup parms() const;

    // Get the set of parameters that will be solved for. The order of the
    // parameters in the ParmGroup matches the order of the coefficients in
    // the normal equations.
    ParmGroup solvables() const;

    // Set the parameters to solve for. Parameters that the measurement
    // expression does not depend on are silently ingnored. Use parms() to find
    // out beforehand which parameters the measurement expression depends on, or
    // use solvables() to get the set of parameters that will be solved for.
    void setSolvables(const ParmGroup &solvables);

    // Notify VisEquator of external updates to the value of the solvables.
    void solvablesChanged();

    // Generate equations.
    template <typename T_ITER>
    T_ITER process(T_ITER first, T_ITER last);

    // Reset processing statistics.
    void clearStats();

    // Dump processing statistics to the provided output stream.
    void dumpStats(ostream &out) const;

private:
    // Nested class that holds the temporary buffers needed while processing
    // a single expression from the set and some counters and timers used to
    // gather processing statistics. This is collected in a nested class such
    // that it's easy to create a thread-private instance for each thread should
    // this code be parallelized. Also, it is a way of grouping a number of
    // members that belong together semantically.
    class ProcContext
    {
    public:
        ProcContext();

        void resize(size_t nCoeff);
        void clearStats();

        // Number of expression specific coefficients.
        size_t                  nCoeff;

        // Mapping from available partial derivatives (parameter, coefficient)
        // to coefficient indidces in the normal matrix.
        vector<unsigned int>    index;
        // References to the (appoximated) partial derivatives.
        vector<Matrix>          partial;
        // Value of the (approximated) partial derivatives.
        vector<double>          partialRe, partialIm;

        // Statistics and timers.
        size_t                  count;

        enum ProcTimer
        {
            EVAL_EXPR,
            EQUATE,
            MAKE_COEFF_MAP,
            TRANSPOSE,
            MAKE_NORM,
            N_ProcTimer
        };

        static string           timerNames[N_ProcTimer];
        NSTimer                 timers[N_ProcTimer];
    };

    // Determine the evaluation grid, i.e. the part of the observation grid
    // that is completely contained in the model domain.
    void makeEvalGrid();

    // Create a mapping for each axis that maps from cells in the evaluation
    // grid to cells in the solution grid.
    void makeCellMap();

    // Create a mapping from (parameter, coefficient) pairs to coefficient
    // indices in the normal equations.
    void makeCoeffMap();

    // Generate a look-up table mapping the coefficients of the parameters that
    // the given element depends on to the respective index in the normal
    // equations.
    void makeElementCoeffMap(const ValueSet &element, ProcContext &context);

    // Insanely complicated boost::multi_array types...
    typedef boost::multi_array<flag_t, 2>::index_range TFRange;
    typedef boost::multi_array<flag_t, 2>::const_array_view<2>::type TFSlice;
    typedef boost::multi_array<flag_t, 4>::index_range FRange;
    typedef boost::multi_array<flag_t, 4>::const_array_view<4>::type FSlice;
    typedef boost::multi_array<sample_t, 4>::index_range SRange;
    typedef boost::multi_array<sample_t, 4>::const_array_view<4>::type SSlice;

    // Generate normal equations for a single expression from the set.
    template <typename T_ITER>
    void procExpr(ProcContext &context, const VisEquator::TFSlice &timeFlag,
        const VisEquator::FSlice &flagLHS, const VisEquator::SSlice &valueLHS,
        const pair<size_t, size_t> &idx, T_ITER out);

    // Create a mapping from cells of axis "from" to cell indices on axis "to".
    // Additionally, the interval of cells of axis "from" that intersect axis
    // "to" (the domain) is returned.
    template <typename T_ITER>
    Interval<size_t> makeAxisMap(const Axis::ShPtr &from,
        const Axis::ShPtr &to, T_ITER out) const;

    // Find the range of cells on the axis that is completely contained inside
    // the given interval.
    Interval<size_t> findContainedCellRange(const Axis::ShPtr &axis,
        const Interval<double> &interval) const;

    // Observed data.
    VisData::Ptr                        itsLHS;

    // Model expressions.
    MeasurementExpr::Ptr                itsRHS;

    // Evaluation grid.
    Grid                                itsEvalGrid;

    // Solution grid.
    Grid                                itsSolGrid;

    // Is the intersection between the observation grid, the model domain, and
    // the solution grid empty? This indicates if whether or not there are
    // visibilities that need to be processed.
    bool                                itsIntersectionEmpty;

    // Location in the observatoin grid of the start of the evaluation grid.
    Location                            itsEvalOffset;

    // Location in the solution grid of the start and end of the evaluation
    // grid.
    Location                            itsEvalStart, itsEvalEnd;

    // Location in the solution grid of the current selection (clipped against
    // the evaluation grid).
    Location                            itsSelectionStart, itsSelectionEnd;

    // The number of cells in the current selection (clipped against the
    // evaluation grid).
    size_t                              itsSelectedCellCount;

    // Location in the solution grid of the current selection relative to the
    // start (in the solution grid) of the evluation grid.
    Location                            itsEvalSelStart, itsEvalSelEnd;

    // Location in the evaluation grid of the current selection.
    Location                            itsEvalReqStart, itsEvalReqEnd;

    // Location in the observation grid of the current selection.
    Location                            itsReqStart, itsReqEnd;

    // Mapping of cells in the evaluation grid to cells in the solution grid.
    vector<size_t>                      itsFreqMap, itsTimeMap;

    // Mapping of baselines and correlations to their respective indices in
    // both observed data and model.
    vector<pair<size_t, size_t> >       itsBlMap, itsCrMap;

    // Total number of coefficients to fit.
    size_t                              itsCoeffCount;

    // Mapping from (parameter, coefficient) pairs to coefficient indices in the
    // condition equations.
    map<PValueKey, unsigned int>        itsCoeffMap;

    // Timer for measuring the execution time of process().
    NSTimer                             itsProcTimer;

    // Expression processing buffers.
    ProcContext                         itsProcContext;
};

// @}

// -------------------------------------------------------------------------- //
// - VisEquator implementation                                              - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
Interval<size_t> VisEquator::makeAxisMap(const Axis::ShPtr &from,
    const Axis::ShPtr &to, T_ITER out) const
{
    Interval<double> overlap(std::max(from->start(), to->start()),
        std::min(from->end(), to->end()));

    if(overlap.start >= overlap.end || casa::near(overlap.start, overlap.end))
    {
        return Interval<size_t>(1, 0);
    }

    Interval<size_t> domain;
    domain.start = from->locate(overlap.start);
    domain.end = from->locate(overlap.end, false, domain.start);

    // Intervals are inclusive by convention.
    const size_t nCells = domain.end - domain.start + 1;
    ASSERT(nCells >= 1);

    // Special case for the first domain cell: lower and possibly upper boundary
    // may be located outside of the overlap between the "from" and "to" axis.
    Interval<size_t> interval;
    interval.start = to->locate(std::max(from->lower(domain.start),
        overlap.start));
    interval.end = to->locate(std::min(from->upper(domain.start), overlap.end),
        false, interval.start);
    for(size_t i = interval.start; i <= interval.end; ++i)
    {
        *out++ = 0;
    }

    for(size_t i = 1; i < nCells - 1; ++i)
    {
        interval.start = to->locate(from->lower(domain.start + i), true,
            interval.end);
        interval.end = to->locate(from->upper(domain.start + i), false,
            interval.end);
        for(size_t j = interval.start; j <= interval.end; ++j)
        {
            *out++ = i;
        }
    }

    if(nCells > 1)
    {
        // Special case for the last domain cell: upper boundary may be located
        // outside of the overlap between the "from" and "to" axis.
        interval.start = to->locate(from->lower(domain.end), true,
            interval.end);
        interval.end = to->locate(std::min(from->upper(domain.end),
            overlap.end), false, interval.end);
        for(size_t i = interval.start; i <= interval.end; ++i)
        {
            *out++ = nCells - 1;
        }
    }

    return domain;
}

template <typename T_ITER>
T_ITER VisEquator::process(T_ITER first, T_ITER last)
{
    DBGASSERT(static_cast<size_t>(distance(first, last)) >= nSelectedCells());

    itsProcTimer.start();

    // Check if there is any data to process and/or coefficients to fit.
    if(isSelectionEmpty() || itsSelectedCellCount == 0 || itsCoeffCount == 0)
    {
        itsProcTimer.stop();
        return first;
    }

    // Initialize CellEquation instances (set correct cell id and clear normal
    // equations).
    CellIterator cellIt(itsSelectionStart, itsSelectionEnd);
    for(T_ITER it = first; it < last; ++it, ++cellIt)
    {
        it->id = itsSolGrid.getCellId(*cellIt);
        it->equation.set(static_cast<unsigned int>(itsCoeffMap.size()));
    }

    TFRange timeTFRange(itsReqStart.second, itsReqEnd.second + 1);
    TFSlice tflag(itsLHS->tslot_flag[boost::indices[TFRange()][timeTFRange]]);

    FRange freqFRange(itsReqStart.first, itsReqEnd.first + 1);
    FRange timeFRange(itsReqStart.second, itsReqEnd.second + 1);
    FSlice flag(itsLHS->vis_flag[boost::indices[FRange()][timeFRange]
        [freqFRange][FRange()]]);

    SRange freqSRange(itsReqStart.first, itsReqEnd.first + 1);
    SRange timeSRange(itsReqStart.second, itsReqEnd.second + 1);
    SSlice sample(itsLHS->vis_data[boost::indices[SRange()][timeSRange]
        [freqSRange][SRange()]]);

    // Construct equations for all baselines.
    for(size_t i = 0; i < itsBlMap.size(); ++i)
    {
        procExpr(itsProcContext, tflag, flag, sample, itsBlMap[i], first);
    }

    itsProcTimer.stop();

    return first + nSelectedCells();
}

template <typename T_ITER>
void VisEquator::procExpr(ProcContext &context,
    const VisEquator::TFSlice &timeFlag, const VisEquator::FSlice &flagLHS,
    const VisEquator::SSlice &valueLHS, const pair<size_t, size_t> &idx,
    T_ITER out)
{
    // Evaluate the right hand side.
    context.timers[ProcContext::EVAL_EXPR].start();
    const JonesMatrix RHS = itsRHS->evaluate(idx.second);
    context.timers[ProcContext::EVAL_EXPR].stop();

    // If the model contains no flags, assume no samples are flagged.
    // TODO: This incurs a cost for models that do not contain flags because
    // a call to virtual FlagArray::operator() is made for each sample.
    FlagArray flagRHS(FlagType(0));
    if(RHS.hasFlags())
    {
        flagRHS = RHS.flags();
    }

    // If all model visibilities are flagged, skip this baseline.
    if(flagRHS.rank() == 0 && flagRHS(0, 0) != 0)
    {
        return;
    }

//    // Construct equations.
//    //
//    // Both LHS and RHS may depend on parameters and any parameter may appear
//    // in both LHS and RHS. Therefore, essentially the model is LHS - RHS (or
//    // alternatively RHS - LHS) and the observables are all zero.
//    //
//    // In the condition equations, the partial derivatives of the model with
//    // respect to the parameters appear with a positive sign. The partial
//    // derivative of LHS - RHS with respect to a parameter p equals:
//    //
//    // (1) d(LHS - RHS)/d(p) = d(LHS)/d(p) - d(RHS)/d(p)
//    //
//    // However, it is often the case that LHS has no associated parameters
//    // (because it represents observed visibility data), while RHS does.
//    // Following equation (1) then requires negation of all the partial
//    // derivatives of RHS. This is avoided by using RHS - LHS as the model
//    // instead of LHS - RHS.

    context.timers[ProcContext::EQUATE].start();

    const size_t nTime = valueLHS.shape()[1];
    const size_t nFreq = valueLHS.shape()[2];

    for(size_t cr = 0; cr < itsCrMap.size(); ++cr)
    {
        const size_t crLHS = itsCrMap[cr].first;
        const size_t crRHS = itsCrMap[cr].second;

        const ValueSet valueSetRHS = RHS.getValueSet(crRHS);

        // If there are no coefficients to fit, continue to the next
        // polarization product.
        if(valueSetRHS.size() == 1)
        {
            continue;
        }

        // Compute the right hand side of the condition equations:
        //
        // 0 - (RHS - LHS) = LHS - RHS
        //
//        Matrix delta = valueSetLHS.value() - valueSetRHS.value();
        Matrix valueRHS = valueSetRHS.value();
//        const double *re, *im;
//        valueSetRHS.value().dcomplexStorage(re, im);

        // Compute the partial derivatives of RHS - LHS with respect to the
        // solvable coefficients and determine a mapping from sequential
        // coefficient number to coefficient index in the condition equations.
        context.timers[ProcContext::MAKE_COEFF_MAP].start();
        makeElementCoeffMap(valueSetRHS, context);
        context.timers[ProcContext::MAKE_COEFF_MAP].stop();

        for(size_t t = 0; t < nTime; ++t)
        {
            if(timeFlag[idx.first][t])
            {
                continue;
            }

            const size_t eqIdx = (itsTimeMap[itsEvalReqStart.second + t]
                - itsEvalSelStart.second) * (itsEvalSelEnd.first
                - itsEvalSelStart.first + 1);

            for(size_t f = 0; f < nFreq; ++f)
            {
                if(flagLHS[idx.first][t][f][crLHS] || flagRHS(f, t))
                {
                    continue;
                }

                casa::LSQFit &equation = (out + eqIdx +
                    (itsFreqMap[itsEvalReqStart.first + f]
                    - itsEvalSelStart.first))->equation;

                // Update statistics.
                ++context.count;

                // Compute right hand side of the equation pair.
                const dcomplex tmp0 = valueLHS[idx.first][t][f][crLHS];
//                const double dre = real(tmp0) - *re++;
//                const double dim = imag(tmp0) - *im++;
                const dcomplex tmp1 = valueRHS.getDComplex(f, t);
                const dcomplex delta = tmp0 - tmp1;

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

                // Generate condition equations.
                context.timers[ProcContext::MAKE_NORM].start();
                equation.makeNorm(context.nCoeff,
                    &(context.index[0]),
                    &(context.partialRe[0]),
                    1.0,
//                    dre);
                    real(delta));

                equation.makeNorm(context.nCoeff,
                    &(context.index[0]),
                    &(context.partialIm[0]),
                    1.0,
//                    dim);
                    imag(delta));
                context.timers[ProcContext::MAKE_NORM].stop();
            }
        }
    }

    context.timers[ProcContext::EQUATE].stop();
}

} //# namespace BBS
} //# namespace LOFAR

#endif
