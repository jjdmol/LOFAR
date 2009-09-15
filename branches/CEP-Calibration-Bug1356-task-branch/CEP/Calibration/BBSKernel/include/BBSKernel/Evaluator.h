//# Evaluator.h: Evaluate a model and assign the result to or subtract/add it
//# from/to the visibility data in the chunk.
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

#ifndef LOFAR_BBSKERNEL_EVALUATOR_H
#define LOFAR_BBSKERNEL_EVALUATOR_H

// \file
// Evaluate a model and assign the result to or subtract/add it from/to the
// visibility data in the chunk.

#include <BBSKernel/Model.h>
#include <BBSKernel/VisData.h>
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

    Evaluator(const VisData::Ptr &chunk, const Model::Ptr &model);

    // Set subset of visibility data to process.
    void setSelection(const vector<baseline_t> &baselines,
        const vector<string> &products);

    // Set operation to perform on the data (equate, subtract, or add).
    void setMode(Mode mode);

    // Evaluate the model and process the model visibilities according to the
    // current processing mode.
    void process();

private:
    struct OpEq
    {
        static void apply(sample_t &lhs, const complex_t &rhs);
    };

    struct OpSub
    {
        static void apply(sample_t &lhs, const complex_t &rhs);
    };

    struct OpAdd
    {
        static void apply(sample_t &lhs, const complex_t &rhs);
    };

    typedef void (Evaluator::*BlProcessor)(const baseline_t &baseline,
        const JonesMatrix &model);

    template <typename T_OPERATOR>
    void blProcess(const baseline_t &baseline, const JonesMatrix &model);

    template <typename T_OPERATOR>
    void blProcessNoFlags(const baseline_t &baseline, const JonesMatrix &model);


    VisData::Ptr        itsChunk;
    Model::Ptr          itsModel;

    vector<baseline_t>  itsBaselines;
    int                 itsProductMask[4];
    BlProcessor         itsBlProcessor[2];

    enum Timer
    {
        ALL,
        MODEL_EVAL,
        APPLY,
        N_Timer
    };

    NSTimer             itsTimers[N_Timer];
    static string       theirTimerNames[N_Timer];
};

// @}

inline void Evaluator::OpEq::apply(sample_t &lhs, const complex_t &rhs)
{
    lhs = rhs;
}

inline void Evaluator::OpSub::apply(sample_t &lhs, const complex_t &rhs)
{
    lhs -= rhs;
}

inline void Evaluator::OpAdd::apply(sample_t &lhs, const complex_t &rhs)
{
    lhs += rhs;
}

template <typename T_OPERATOR>
void Evaluator::blProcess(const baseline_t &baseline, const JonesMatrix &model)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    const size_t bl = dims.getBaselineIndex(baseline);
    const size_t nFreq = dims.getChannelCount();
    const size_t nTime = dims.getTimeslotCount();

    const JonesMatrix::View coherence(model.view());

    // Process visibilities for this baseline.
    for(unsigned int i = 0; i < 4; ++i)
    {
        // Check if the chunk contains the current correlation product.
        const int extProd = itsProductMask[i];
        if(extProd == -1)
        {
            continue;
        }

        // Get pointers to the model visibilities.
        const Matrix &modelVis = coherence(i / 2, i % 2);
        const double *re = 0, *im = 0;
        modelVis.dcomplexStorage(re, im);

        // Determine pointer increments (0 for scalar model visibility).
        const size_t step = modelVis.isArray() ? 1 : 0;
        DBGASSERT(!modelVis.isArray()
            || static_cast<size_t>(modelVis.nelements()) == nFreq * nTime);

        // Get random access iterator for the flags.
        const FlagArray &flags = model.flags();
        FlagArray::const_iterator flagIt = flags.begin();

        // Determine pointer increments (0 for scalar flags).
        const size_t flagStep = flags.rank() == 0 ? 0 : 1;
        DBGASSERT(flags.rank() == 0 || flags.size() == nFreq * nTime);

        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range DRange;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type DView;
        DView obsVis(itsChunk->vis_data[boost::indices[bl][DRange()]
            [DRange()][extProd]]);

        typedef boost::multi_array<flag_t, 4>::index_range FRange;
        typedef boost::multi_array<flag_t, 4>::array_view<2>::type FView;
        FView obsFlags(itsChunk->vis_flag[boost::indices[bl][FRange()]
            [FRange()][extProd]]);

        // Process visibilities and flags.
        for(size_t ts = 0; ts < nTime; ++ts)
        {
            for(size_t ch = 0; ch < nFreq; ++ch)
            {
                obsFlags[ts][ch] |= *flagIt;
                flagIt += flagStep;

                T_OPERATOR::apply(obsVis[ts][ch], sample_t(*re, *im));
                re += step;
                im += step;
            }
        }
    }
}

template <typename T_OPERATOR>
void Evaluator::blProcessNoFlags(const baseline_t &baseline,
    const JonesMatrix &model)
{
    const VisDimensions &dims = itsChunk->getDimensions();
    const size_t bl = dims.getBaselineIndex(baseline);
    const size_t nFreq = dims.getChannelCount();
    const size_t nTime = dims.getTimeslotCount();

    const JonesMatrix::View coherence(model.view());

    for(unsigned int i = 0; i < 4; ++i)
    {
        // Check if the chunk contains the current correlation product.
        const int extProd = itsProductMask[i];
        if(extProd == -1)
        {
            continue;
        }

        // Get pointers to the model visibilities.
        const Matrix modelVis = coherence(i / 2, i % 2);
        const double *re = 0, *im = 0;
        modelVis.dcomplexStorage(re, im);

        // Determine pointer increments (0 for scalar model visibility).
        const size_t step = modelVis.isArray() ? 1 : 0;
        DBGASSERT(!modelVis.isArray()
            || static_cast<size_t>(modelVis.nelements()) == nFreq * nTime);

        // Get a view on the relevant slice of the chunk.
        typedef boost::multi_array<sample_t, 4>::index_range DRange;
        typedef boost::multi_array<sample_t, 4>::array_view<2>::type DView;
        DView obsVis(itsChunk->vis_data[boost::indices[bl][DRange()]
            [DRange()][extProd]]);

        // Process visibilities.
        for(size_t ts = 0; ts < nTime; ++ts)
        {
            for(size_t ch = 0; ch < nFreq; ++ch)
            {
                T_OPERATOR::apply(obsVis[ts][ch], sample_t(*re, *im));
                re += step;
                im += step;
            }
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
