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

#include <BBSKernel/VisData.h>
#include <BBSKernel/MeasurementExpr.h>

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

    Evaluator(const VisData::Ptr &lhs, const MeasurementExpr::Ptr &rhs);

    // Select baselines in [first, last) for processing.
    template <typename T_ITER>
    void setBaselines(T_ITER first, T_ITER last);

    // Select correlations in [first, last) for processing.
    template <typename T_ITER>
    void setCorrelations(T_ITER first, T_ITER last);

    // Set operation to perform on the data (equate, subtract, or add).
    void setMode(Mode mode);

    // Evaluate the expressions in the set and process the visibilities
    // according to the current processing mode.
    void process();

    // Reset processing statistics.
    void clearStats();

    // Dump processing statistics to the logger.
    void dumpStats() const;

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

    // Create a mapping of baselines known by both LHS and RHS to their
    // respective indices.
    void initBaselineMap();
    template <typename T_ITER>
    void makeBaselineMap(T_ITER first, T_ITER last);

    // Create a mapping of correlations known by both LHS and RHS to their
    // respective indices.
    void initCorrelationMap();
    template <typename T_ITER>
    void makeCorrelationMap(T_ITER first, T_ITER last);

    // Signature of sample processor function.
    typedef void (Evaluator::*ExprProcessor)(size_t &bl,
        const JonesMatrix &rhs);

    template <typename T_OPERATOR>
    void procExprWithFlags(size_t &bl, const JonesMatrix &rhs);

    template <typename T_OPERATOR>
    void procExpr(size_t &bl, const JonesMatrix &rhs);

    // Visibility data buffer.
    VisData::Ptr                        itsLHS;
    // Measurement equation.
    MeasurementExpr::Ptr                itsRHS;

    // Mapping of baselines and correlations to their respective indices in
    // both observed data and model.
    vector<pair<size_t, size_t> >       itsBlMap, itsCrMap;

    ExprProcessor                       itsExprProcessor[2];

    // Timers.
    enum ProcTimer
    {
        ALL,
        EVAL_RHS,
        APPLY,
        N_ProcTimer
    };

    NSTimer                             itsProcTimers[N_ProcTimer];
    static string                       theirProcTimerNames[N_ProcTimer];
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

template <typename T_ITER>
void Evaluator::setBaselines(T_ITER first, T_ITER last)
{
    if(first != last)
    {
        itsBlMap.clear();
        makeBaselineMap(first, last);
    }
}

template <typename T_ITER>
void Evaluator::setCorrelations(T_ITER first, T_ITER last)
{
    if(first != last)
    {
        itsCrMap.clear();
        makeCorrelationMap(first, last);
    }
}

template <typename T_ITER>
void Evaluator::makeBaselineMap(T_ITER first, T_ITER last)
{
    const BaselineSeq &blLHS = itsLHS->baselines();
    const BaselineSeq &blRHS = itsRHS->baselines();

    for(; first != last; ++first)
    {
        const size_t lhs = blLHS.index(*first);
        const size_t rhs = blRHS.index(*first);
        if(lhs != blLHS.size() && rhs != blRHS.size())
        {
            itsBlMap.push_back(make_pair(lhs, rhs));
        }
    }
}

template <typename T_ITER>
void Evaluator::makeCorrelationMap(T_ITER first, T_ITER last)
{
    const CorrelationSeq &crLHS = itsLHS->correlations();
    const CorrelationSeq &crRHS = itsRHS->correlations();

    for(; first != last; ++first)
    {
        const size_t lhs = crLHS.index(*first);
        const size_t rhs = crRHS.index(*first);
        if(lhs != crLHS.size() && rhs != crRHS.size())
        {
            itsCrMap.push_back(make_pair(lhs, rhs));
        }
    }
}

template <typename T_OPERATOR>
void Evaluator::procExprWithFlags(size_t &bl, const JonesMatrix &rhs)
{
    // Determine no. of samples along frequency and time axis.
    const size_t nFreq = itsLHS->grid()[FREQ]->size();
    const size_t nTime = itsLHS->grid()[TIME]->size();

    // Determine flag iterator increments (0 for scalar flags).
    const FlagArray flagsRHS = rhs.flags();
    const size_t flagStep = flagsRHS.rank() == 0 ? 0 : 1;
    DBGASSERT(flagsRHS.rank() == 0 || flagsRHS.size() == nFreq * nTime);

    for(size_t cr = 0; cr < itsCrMap.size(); ++cr)
    {
        const size_t crLHS = itsCrMap[cr].first;
        const size_t crRHS = itsCrMap[cr].second;

        // Get random access iterator for the flags.
        FlagArray::const_iterator flagIt = flagsRHS.begin();

        // Get pointers to the computed visibilities.
        const Matrix samplesRHS = rhs.getValueSet(crRHS).value();
        const double *reIt = 0, *imIt = 0;
        samplesRHS.dcomplexStorage(reIt, imIt);

        // Determine pointer increments (0 for scalar).
        const size_t sampleStep = samplesRHS.isArray() ? 1 : 0;
        DBGASSERT(!samplesRHS.isArray()
            || static_cast<size_t>(samplesRHS.nelements()) == nFreq * nTime);

        // Get a view on the relevant slice of the data buffer.
        typedef boost::multi_array<flag_t, 4>::index_range FRange;
        typedef boost::multi_array<flag_t, 4>::array_view<2>::type FSlice;
        FSlice flagsLHS(itsLHS->vis_flag[boost::indices[bl][FRange()][FRange()]
            [crLHS]]);

        typedef boost::multi_array<sample_t, 4>::index_range SRange;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type SSlice;
        SSlice samplesLHS(itsLHS->vis_data[boost::indices[bl][SRange()]
            [SRange()][crLHS]]);

        // Process visibilities and flags.
        for(size_t t = 0; t < nTime; ++t)
        {
            for(size_t f = 0; f < nFreq; ++f)
            {
                flagsLHS[t][f] |= *flagIt;
                T_OPERATOR::apply(samplesLHS[t][f], makedcomplex(*reIt, *imIt));

                flagIt += flagStep;
                reIt += sampleStep;
                imIt += sampleStep;
            }
        }
    }
}

template <typename T_OPERATOR>
void Evaluator::procExpr(size_t &bl, const JonesMatrix &rhs)
{
    // Determine no. of samples along frequency and time axis.
    const size_t nFreq = itsLHS->grid()[FREQ]->size();
    const size_t nTime = itsLHS->grid()[TIME]->size();

    for(size_t cr = 0; cr < itsCrMap.size(); ++cr)
    {
        const size_t crLHS = itsCrMap[cr].first;
        const size_t crRHS = itsCrMap[cr].second;

        // Get pointers to the computed visibilities.
        const Matrix samplesRHS = rhs.getValueSet(crRHS).value();
        const double *reIt = 0, *imIt = 0;
        samplesRHS.dcomplexStorage(reIt, imIt);

        // Determine pointer increments (0 for scalar).
        const size_t sampleStep = samplesRHS.isArray() ? 1 : 0;
        DBGASSERT(!samplesRHS.isArray()
            || static_cast<size_t>(samplesRHS.nelements()) == nFreq * nTime);

        // Get a view on the relevant slice of the data buffer.
        typedef boost::multi_array<sample_t, 4>::index_range SRange;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type SSlice;
        SSlice samplesLHS(itsLHS->vis_data[boost::indices[bl][SRange()]
            [SRange()][crLHS]]);

        // Process visibilities.
        for(size_t t = 0; t < nTime; ++t)
        {
            for(size_t f = 0; f < nFreq; ++f)
            {
                T_OPERATOR::apply(samplesLHS[t][f], makedcomplex(*reIt, *imIt));
                reIt += sampleStep;
                imIt += sampleStep;
            }
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
