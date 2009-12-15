//# Equator.h: Generate normal equations that tie a model to an observation.
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

#ifndef LOFAR_BBSKERNEL_EQUATOR_H
#define LOFAR_BBSKERNEL_EQUATOR_H

// \file
// Generate normal equations that tie a model to an observation.

#include <BBSKernel/ExprSet.h>
#include <BBSKernel/SolverInterfaceTypes.h>
#include <BBSKernel/Types.h>
#include <BBSKernel/Expr/ExprResult.h>

#include <Common/Timer.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Equator
{
public:
    Equator(const ExprSet<JonesMatrix>::Ptr &lhs,
        const ExprSet<JonesMatrix>::Ptr &rhs, const Grid &evalGrid,
        const Grid &solGrid, const CoeffIndex &coeffIndex);

    // Set the (inclusive) range of cells of the solution grid to process.
    void setCellSelection(const Location &start, const Location &end);

    // Generate equations.
    void process(vector<CellEquation> &out);

private:
    // Nested class that represents an interval of cells along an axis. Used to
    // create the mapping from cells in the solution grid to intervals of cells
    // in the observation grid.
    class Interval
    {
    public:
        Interval();

        unsigned int    start;
        unsigned int    end;
    };

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

        void resize(unsigned int nCoeff);
        void reset();

        // Number of expression specific coefficients.
        size_t                  nCoeff;

        // Mapping from available partial derivatives (parmId, coeffId) to a
        // coefficient index in the normal matrix.
        vector<unsigned int>    index;
        // References to the (appoximated) partial derivatives.
        vector<Matrix>          partial;
        // Value of the (approximated) partial derivatives.
        vector<double>          partialRe, partialIm;

        // Statistics and timers.
        size_t                  count;

        enum ProcTimer
        {
            EVAL_LHS,
            EVAL_RHS,
            MERGE_FLAGS,
            EQUATE,
            MAKE_COEFF_MAP,
            TRANSPOSE,
            MAKE_NORM,
            N_ProcTimer
        };

        static string           timerNames[N_ProcTimer];
        NSTimer                 timers[N_ProcTimer];
    };

    // Create a mapping for each axis that maps from cells in the solution grid
    // to cell intervals in the evaluation grid.
    void makeGridMapping();

    // Create a mapping from cells of axis "from" to cell intervals on axis
    // "to". Additionally, the interval of cells of axis "from" that intersect
    // axis "to" (the domain) is returned.
    pair<Interval, vector<Interval> > makeAxisMapping(const Axis::ShPtr &from,
        const Axis::ShPtr &to) const;

    // Create a mapping from (parmId, coeffId) to a coefficient index in the
    // condition equations.
    void makeCoeffMapping(const CoeffIndex &index);

    // Compute partial derivatives and generate a look-up table for the index of
    // the corresponding coefficients in the condition equations for a single
    // expression from the set.
    void makeExprCoeffMapping(const ValueSet &lhs, const ValueSet &rhs,
        ProcContext &context);

    // Generate normal equations for a single expression from the set.
    template <typename T_ITER>
    void procExpr(T_ITER result, size_t idx, ProcContext &context);

    // Sets of expressions between which the difference is to be minimized.
    ExprSet<JonesMatrix>::Ptr     itsLHS;
    ExprSet<JonesMatrix>::Ptr     itsRHS;

    // Evaluation grid.
    Grid                                itsEvalGrid;
    // Solution grid.
    Grid                                itsSolGrid;

    // Is the intersection between the solution grid and the evaluation grid
    // empty?
    bool                                itsIntersectionEmpty;
    // Location in the solution grid of the start and end of the evaluation
    // grid.
    Location                            itsEvalStart, itsEvalEnd;
    // Location in the solution grid of the current selection (clipped against
    // the evaluation grid).
    Location                            itsSelectionStart, itsSelectionEnd;
    // The number of cells in the current selection (clipped against the
    // evaluation grid).
    unsigned int                        itsSelectedCellCount;
    // Location in the solution grid of the current selection relative to the
    // start (in the solution grid) of the evaluation grid.
    Location                            itsEvalSelStart, itsEvalSelEnd;
    // Location in the evaluation grid of the current selection.
    Location                            itsReqStart, itsReqEnd;

    // Mapping of cells in the solution grid to intervals of cells along the
    // observation grid's axes.
    vector<Interval>                    itsFreqIntervals, itsTimeIntervals;

    // Mapping from (parmId, coeffId) to a coefficient index in the condition
    // equations.
    map<PValueKey, unsigned int>        itsCoeffMap;

    // Expression processing buffers.
    ProcContext                         itsProcContext;
};

// @}

// -------------------------------------------------------------------------- //
// - Equator implementation                                                 - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
void Equator::procExpr(T_ITER result, size_t idx, ProcContext &context)
{
    // Evaluate the left hand side.
    context.timers[ProcContext::EVAL_LHS].start();
    const JonesMatrix LHS = itsLHS->evaluate(idx);
    context.timers[ProcContext::EVAL_LHS].stop();

    // Evaluate the right hand side.
    context.timers[ProcContext::EVAL_RHS].start();
    const JonesMatrix RHS = itsRHS->evaluate(idx);
    context.timers[ProcContext::EVAL_RHS].stop();

    // If the model contains no flags, assume no samples are flagged.
    // TODO: This incurs a cost for models that do not contain flags because
    // a call to virtual FlagArray::operator() is made for each sample.
    context.timers[ProcContext::MERGE_FLAGS].start();

    FlagArray flags(FlagType(0));

    if(LHS.hasFlags())
    {
        flags = flags | LHS.flags();
    }

    if(RHS.hasFlags())
    {
        flags = flags | RHS.flags();
    }
    context.timers[ProcContext::MERGE_FLAGS].stop();

    // Construct equations.
    //
    // Both LHS and RHS may depend on parameters and any parameter may appear
    // in both LHS and RHS. Therefore, essentially the model is LHS - RHS (or
    // alternatively RHS - LHS) and the observables are all zero.
    //
    // In the condition equations, the partial derivatives of the model with
    // respect to the parameters appear with a positive sign. The partial
    // derivative of LHS - RHS with respect to a parameter p equals:
    //
    // (1) d(LHS - RHS)/d(p) = d(LHS)/d(p) - d(RHS)/d(p)
    //
    // However, it is often the case that LHS has no associated parameters
    // (because it represents observed visibility data), while RHS does.
    // Following equation (1) then requires negation of all the partial
    // derivatives of RHS. This is avoided by using RHS - LHS as the model
    // instead of LHS - RHS.

    context.timers[ProcContext::EQUATE].start();

    // Offset in the evaluation grid of the start of the selected solution
    // cells.
    const Location visStart(itsFreqIntervals[itsEvalSelStart.first].start,
        itsTimeIntervals[itsEvalSelStart.second].start);

    for(unsigned int el = 0; el < 4; ++el)
    {
        const ValueSet valueSetLHS = LHS.getValueSet(el);
        const ValueSet valueSetRHS = RHS.getValueSet(el);

        // If there are no coefficients to fit, continue to the next
        // polarization product.
        if(valueSetLHS.size() == 1 && valueSetRHS.size() == 1)
        {
            continue;
        }

        // Compute the right hand side of the condition equations:
        //
        // 0 - (RHS - LHS) = LHS - RHS
        //
        Matrix delta = valueSetLHS.value() - valueSetRHS.value();

        // Compute the partial derivatives of RHS - LHS with respect to the
        // solvable coefficients and determine a mapping from sequential
        // coefficient number to coefficient index in the condition equations.
        context.timers[ProcContext::MAKE_COEFF_MAP].start();
        makeExprCoeffMapping(valueSetLHS, valueSetRHS, context);
        context.timers[ProcContext::MAKE_COEFF_MAP].stop();

        // Loop over all selected cells and generate equations, adding them
        // to the associated normal matrix.
        T_ITER cellEqIt = result;
        CellIterator cellIt(itsEvalSelStart, itsEvalSelEnd);
        while(!cellIt.atEnd())
        {
            // Get a reference to the normal matrix associated to this cell.
            casa::LSQFit &equation = cellEqIt->equation;

            // Samples to process (observed visibilities).
            const Interval &chInterval = itsFreqIntervals[cellIt->first];
            const Interval &tsInterval = itsTimeIntervals[cellIt->second];

            for(unsigned int ts = tsInterval.start; ts <= tsInterval.end; ++ts)
            {
                // Timeslot index relative to the start of the selection in
                // evaluation grid coordinates.
                int tsRel = static_cast<int>(ts - visStart.second);

                // Construct two equations for each unflagged sample.
                for(unsigned int ch = chInterval.start; ch <= chInterval.end;
                    ++ch)
                {
                    // Channel index relative to the start of the selection in
                    // evaluation grid coordinates.
                    int chRel = static_cast<int>(ch - visStart.first);

                    if(flags(chRel, tsRel))
                    {
                        continue;
                    }

                    // Update statistics.
                    ++context.count;

                    // Load right hand side of the equation pair.
                    const dcomplex sampleDelta =
                        delta.getDComplex(chRel, tsRel);

                    // Tranpose the partial derivatives.
                    context.timers[ProcContext::TRANSPOSE].start();
                    for(unsigned int i = 0; i < context.nCoeff; ++i)
                    {
                        const dcomplex partial =
                            context.partial[i].getDComplex(chRel, tsRel);

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
                        real(sampleDelta));

                    equation.makeNorm(context.nCoeff,
                        &(context.index[0]),
                        &(context.partialIm[0]),
                        1.0,
                        imag(sampleDelta));
                    context.timers[ProcContext::MAKE_NORM].stop();

                    // Move to the next channel.
                    ++chRel;
                } // End of loop over frequency.

                // Move to next timeslot.
                ++tsRel;
            } // End of loop over time.

            // Move to the next solution cell.
            ++cellEqIt;
            ++cellIt;
        }
    }

    context.timers[ProcContext::EQUATE].stop();
}

} //# namespace BBS
} //# namespace LOFAR

#endif
