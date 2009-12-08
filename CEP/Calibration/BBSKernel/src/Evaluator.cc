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

#include <lofar_config.h>

#include <BBSKernel/Evaluator.h>
#include <BBSKernel/Exceptions.h>

namespace LOFAR
{
namespace BBS
{

string Evaluator::theirTimerNames[Evaluator::N_Timer] =
    {"ALL",
    "EVAL_RHS",
    "APPLY"};

Evaluator::Evaluator(const VisData::Ptr &chunk,
    const ExprSet<JonesMatrix>::Ptr &expr)
    :   itsChunk(chunk),
        itsExprSet(expr)
{
    // Set default processing mode.
    setMode(EQUATE);

    // Set default visibility selection.
    const VisDimensions &dims = itsChunk->getDimensions();
    setSelection(dims.getBaselines(), dims.getPolarizations());

    // Set request grid.
    itsExprSet->setEvalGrid(itsChunk->getDimensions().getGrid());
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
        itsExprProcessor[0] = &Evaluator::procExprWithFlags<OpEq>;
        itsExprProcessor[1] = &Evaluator::procExpr<OpEq>;
        break;
    case SUBTRACT:
        itsExprProcessor[0] = &Evaluator::procExprWithFlags<OpSub>;
        itsExprProcessor[1] = &Evaluator::procExpr<OpSub>;
        break;
    case ADD:
        itsExprProcessor[0] = &Evaluator::procExprWithFlags<OpAdd>;
        itsExprProcessor[1] = &Evaluator::procExpr<OpAdd>;
        break;
    default:
        THROW(BBSKernelException, "Invalid mode specified.");
    }
}

void Evaluator::process()
{
    // Reset statistics.
    for(size_t i = 0; i < Evaluator::N_Timer; ++i)
    {
        itsTimers[i].reset();
    }

    itsTimers[ALL].start();
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        const baseline_t &baseline = itsBaselines[i];

//        LOG_DEBUG_STR("Baseline: " << baseline.first << " - "
//            << baseline.second);

        // Evaluate the expression for this baseline.
        itsTimers[EVAL_RHS].start();
        const JonesMatrix expr = itsExprSet->evaluate(i);
        itsTimers[EVAL_RHS].stop();

        itsTimers[APPLY].start();
        // Process the visibilities according to the current processing mode.
        if(expr.hasFlags())
        {
            const FlagArray flags = expr.flags();
            if(flags.rank() > 0 || flags(0, 0) != 0)
            {
                (this->*itsExprProcessor[0])(baseline, expr);
            }
            else
            {
                // Optimization: If the flags of the expression are scalar and
                // equal to zero (false) then they can be ignored.
                (this->*itsExprProcessor[1])(baseline, expr);
            }
        }
        else
        {
            (this->*itsExprProcessor[1])(baseline, expr);
        }
        itsTimers[APPLY].stop();
    }
    itsTimers[ALL].stop();

    // Print statistics.
    for(size_t i = 0; i < Evaluator::N_Timer; ++i)
    {
        const double elapsed = itsTimers[i].getElapsed() * 1e3;
        const unsigned long long count = itsTimers[i].getCount();
        LOG_DEBUG_STR("TIMER ms " << Evaluator::theirTimerNames[i] << " total "
            << elapsed << " count " << count << " avg " << elapsed / count);
    }
}

} //# namespace BBS
} //# namespace LOFAR
