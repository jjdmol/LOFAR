//# Evaluator.h: Evaluate an expression and assign, subtract, or add the result
//# to / from a buffer of visibility data.
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

#ifndef LOFAR_BBSKERNEL_EVALUATOR_H
#define LOFAR_BBSKERNEL_EVALUATOR_H

// \file
// Evaluator.h: Evaluate an expression and assign, subtract, or add the result
// to / from a buffer of visibility data.

#include <BBSKernel/ExprSet.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/Expr/ExprResult.h>

#include <Common/Timer.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Evaluator
{
public:
    enum Mode
    {
        EQUATE = 0,
        SUBTRACT,
        ADD,
        N_Mode
    };

    Evaluator(const VisData::Ptr &chunk,
        const ExprSet<JonesMatrix>::Ptr &expr);

    // Set subset of visibility data to process.
    void setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products);

    // Set operation to perform on the data (equate, subtract, or add).
    void setMode(Mode mode);

    // Evaluate the expressions in the set and process the visibilities
    // according to the current processing mode.
    void process();

private:
    struct OpEq
    {
        static void apply(sample_t &lhs, const dcomplex &rhs);
    };

    struct OpSub
    {
        static void apply(sample_t &lhs, const dcomplex &rhs);
    };

    struct OpAdd
    {
        static void apply(sample_t &lhs, const dcomplex &rhs);
    };

    typedef void (Evaluator::*ExprProcessor)(const baseline_t &baseline,
        const JonesMatrix &expr);

    template <typename T_OPERATOR>
    void procExprWithFlags(const baseline_t &baseline,
        const JonesMatrix &expr);

    template <typename T_OPERATOR>
    void procExpr(const baseline_t &baseline, const JonesMatrix &expr);


    VisData::Ptr                    itsChunk;
    ExprSet<JonesMatrix>::Ptr itsExprSet;

    vector<baseline_t>              itsBaselines;
    int                             itsProductMask[4];
    ExprProcessor                   itsExprProcessor[2];

    enum Timer
    {
        ALL,
        EVAL_RHS,
        APPLY,
        N_Timer
    };

    NSTimer             itsTimers[N_Timer];
    static string       theirTimerNames[N_Timer];
};

// @}

// -------------------------------------------------------------------------- //
// - Evaluator implementation                                               - //
// -------------------------------------------------------------------------- //

inline void Evaluator::OpEq::apply(sample_t &lhs, const dcomplex &rhs)
{
    lhs = rhs;
}

inline void Evaluator::OpSub::apply(sample_t &lhs, const dcomplex &rhs)
{
    lhs -= rhs;
}

inline void Evaluator::OpAdd::apply(sample_t &lhs, const dcomplex &rhs)
{
    lhs += rhs;
}

template <typename T_OPERATOR>
void Evaluator::procExprWithFlags(const baseline_t &baseline,
    const JonesMatrix &expr)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    const size_t bl = dims.getBaselineIndex(baseline);
    const size_t nFreq = dims.getChannelCount();
    const size_t nTime = dims.getTimeslotCount();

    // Process visibilities for this baseline.
    for(unsigned int el = 0; el < 4; ++el)
    {
        // Check if the chunk contains the current correlation product.
        const int extProd = itsProductMask[el];
        if(extProd == -1)
        {
            continue;
        }

        // Get pointers to the computed visibilities.
        const ValueSet exprValue = expr.getValueSet(el);
        const Matrix value = exprValue.value();
        const double *re = 0, *im = 0;
        value.dcomplexStorage(re, im);

        // Determine pointer increments (0 for scalar).
        const size_t step = value.isArray() ? 1 : 0;
        DBGASSERT(!value.isArray()
            || static_cast<size_t>(value.nelements()) == nFreq * nTime);

        // Get random access iterator for the flags.
        const FlagArray &flags = expr.flags();
        FlagArray::const_iterator flagIt = flags.begin();

        // Determine pointer increments (0 for scalar flags).
        const size_t flagStep = flags.rank() == 0 ? 0 : 1;
        DBGASSERT(flags.rank() == 0 || flags.size() == nFreq * nTime);

        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range DRange;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type DView;
        DView chunkVis(itsChunk->vis_data[boost::indices[bl][DRange()]
            [DRange()][extProd]]);

        typedef boost::multi_array<flag_t, 4>::index_range FRange;
        typedef boost::multi_array<flag_t, 4>::array_view<2>::type FView;
        FView chunkFlags(itsChunk->vis_flag[boost::indices[bl][FRange()]
            [FRange()][extProd]]);

        // Process visibilities and flags.
        for(size_t ts = 0; ts < nTime; ++ts)
        {
            for(size_t ch = 0; ch < nFreq; ++ch)
            {
                chunkFlags[ts][ch] |= *flagIt;
                flagIt += flagStep;

                T_OPERATOR::apply(chunkVis[ts][ch], makedcomplex(*re, *im));
                re += step;
                im += step;
            }
        }
    }
}

template <typename T_OPERATOR>
void Evaluator::procExpr(const baseline_t &baseline, const JonesMatrix &expr)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    const size_t bl = dims.getBaselineIndex(baseline);
    const size_t nFreq = dims.getChannelCount();
    const size_t nTime = dims.getTimeslotCount();

    // Process visibilities for this baseline.
    for(unsigned int el = 0; el < 4; ++el)
    {
        // Check if the chunk contains the current correlation product.
        const int extProd = itsProductMask[el];
        if(extProd == -1)
        {
            continue;
        }

        // Get pointers to the computed visibilities.
        const ValueSet exprValue = expr.getValueSet(el);
        const Matrix value = exprValue.value();
        const double *re = 0, *im = 0;
        value.dcomplexStorage(re, im);

        // Determine pointer increments (0 for scalar).
        const size_t step = value.isArray() ? 1 : 0;
        DBGASSERT(!value.isArray()
            || static_cast<size_t>(value.nelements()) == nFreq * nTime);

        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range DRange;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type DView;
        DView chunkVis(itsChunk->vis_data[boost::indices[bl][DRange()]
            [DRange()][extProd]]);

        // Process visibilities and flags.
        for(size_t ts = 0; ts < nTime; ++ts)
        {
            for(size_t ch = 0; ch < nFreq; ++ch)
            {
                T_OPERATOR::apply(chunkVis[ts][ch], makedcomplex(*re, *im));
                re += step;
                im += step;
            }
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
