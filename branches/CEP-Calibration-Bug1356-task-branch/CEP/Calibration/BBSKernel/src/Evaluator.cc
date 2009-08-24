//# Evaluator.cc: Evaluate a model and assign the result to or subtract it from
//# the visibility data in the chunk.
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
#include <BBSKernel/Evaluator.h>
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/Expr/MatrixComplexArr.h>

namespace LOFAR
{
namespace BBS
{

Evaluator::Evaluator(const VisData::Ptr &chunk, const Model::Ptr &model)
    :   itsChunk(chunk),
        itsModel(model)
{
    // Set defaults.
    setMode(EQUATE);

    const VisDimensions &dims = itsChunk->getDimensions();
    setSelection(dims.getBaselines(), dims.getPolarizations());
}

void Evaluator::setSelection(const vector<baseline_t> &baselines,
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

void Evaluator::setMode(Mode mode)
{
    switch(mode)
    {
    case EQUATE:
        itsBlProcessor[0] = &Evaluator::blProcess<OpEq>;
        itsBlProcessor[1] = &Evaluator::blProcessNoFlags<OpEq>;
        break;
    case SUBTRACT:
        itsBlProcessor[0] = &Evaluator::blProcess<OpSub>;
        itsBlProcessor[1] = &Evaluator::blProcessNoFlags<OpSub>;
        break;
    case ADD:
        itsBlProcessor[0] = &Evaluator::blProcess<OpAdd>;
        itsBlProcessor[1] = &Evaluator::blProcessNoFlags<OpAdd>;
        break;
    default:
        THROW(BBSKernelException, "Invalid mode specified.");
    }
}

void Evaluator::process()
{
    // Create a request.
    itsModel->setRequestGrid(itsChunk->getDimensions().getGrid());

    itsTimers[MODEL_EVAL].reset();
    itsTimers[PROCESS].reset();

    itsTimers[PROCESS].start();
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        const baseline_t baseline = itsBaselines[i];
        LOG_DEBUG_STR("" << baseline.first << "-" << baseline.second);

        // Evaluate the model.
        itsTimers[MODEL_EVAL].start();
        const JonesMatrix result = itsModel->evaluate(baseline);
        itsTimers[MODEL_EVAL].stop();

        // Process the visibilities according to the current processing mode.
        if(result.hasFlags())
        {
            const FlagArray &flags = result.flags();
            if(flags.rank() > 0 || flags(0, 0) != 0)
            {
                (this->*itsBlProcessor[0])(baseline, result);
            }
            else
            {
                // Optimization: If the flags of the result are scalar and equal
                // to zero then they can be ignored.
                (this->*itsBlProcessor[1])(baseline, result);
            }
        }
        else
        {
            (this->*itsBlProcessor[1])(baseline, result);
        }
    }
    itsTimers[PROCESS].stop();

    LOG_DEBUG("Timings:");
    LOG_DEBUG_STR("> Model evaluation: " << itsTimers[MODEL_EVAL]);
    LOG_DEBUG_STR("> Process: " << itsTimers[PROCESS]);
    LOG_DEBUG_STR("CLONE COUNT: " << MatrixComplexArr::clone_count);
}

} //# namespace BBS
} //# namespace LOFAR
