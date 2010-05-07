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

string Evaluator::theirProcTimerNames[Evaluator::N_ProcTimer] =
    {"ALL",
    "EVAL_RHS",
    "APPLY"};

Evaluator::Evaluator(const VisData::Ptr &lhs, const MeasurementExpr::Ptr &rhs)
    :   itsLHS(lhs),
        itsRHS(rhs)
{
    // Construct a sequence of pairs of indices of matching baselines (i.e.
    // baselines known by both LHS and RHS).
    makeIndexMap(itsLHS->baselines(), itsRHS->baselines(),
        back_inserter(itsBlMap));

    if(itsBlMap.empty())
    {
        LOG_WARN_STR("No baselines found for which data is available in both"
            " the observation and the model.");
    }

    // Construct a sequence of pairs of indices of matching correlations (i.e.
    // correlations known by both LHS and RHS).
    makeIndexMap(itsLHS->correlations(), itsRHS->correlations(),
        back_inserter(itsCrMap));

    if(itsCrMap.empty())
    {
        LOG_WARN_STR("No correlations found for which data is available in both"
            " the observation and the model.");
    }

    // Set default processing mode.
    setMode(EQUATE);

    // Set request grid.
    // TODO: More robust checks on expression domain.
    itsRHS->setEvalGrid(itsLHS->grid());
}

void Evaluator::setBaselineMask(const BaselineMask &mask)
{
    itsBlMap.clear();
    makeIndexMap(itsLHS->baselines(), itsRHS->baselines(), mask,
        back_inserter(itsBlMap));
}

void Evaluator::setCorrelationMask(const CorrelationMask &mask)
{
    itsCrMap.clear();
    makeIndexMap(itsLHS->correlations(), itsRHS->correlations(), mask,
        back_inserter(itsCrMap));
}

bool Evaluator::isSelectionEmpty() const
{
    return itsBlMap.empty() || itsCrMap.empty();
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
        THROW(BBSKernelException, "Invalid processing mode specified.");
    }
}

void Evaluator::process()
{
    itsProcTimers[ALL].start();

    if(isSelectionEmpty())
    {
        itsProcTimers[ALL].stop();
        return;
    }

    for(size_t i = 0; i < itsBlMap.size(); ++i)
    {
        // Evaluate the expression for this baseline.
        itsProcTimers[EVAL_RHS].start();
        const JonesMatrix rhs = itsRHS->evaluate(itsBlMap[i].second);
        itsProcTimers[EVAL_RHS].stop();

        itsProcTimers[APPLY].start();
        // Process the visibilities according to the current processing mode.
        if(rhs.hasFlags())
        {
            const FlagArray flags = rhs.flags();
            if(flags.rank() > 0 || flags(0, 0) != 0)
            {
                (this->*itsExprProcessor[0])(itsBlMap[i].first, rhs);
            }
            else
            {
                // Optimization: If the flags of the expression are scalar and
                // equal to zero (false) then they can be ignored.
                (this->*itsExprProcessor[1])(itsBlMap[i].first, rhs);
            }
        }
        else
        {
            (this->*itsExprProcessor[1])(itsBlMap[i].first, rhs);
        }
        itsProcTimers[APPLY].stop();
    }
    itsProcTimers[ALL].stop();
}

void Evaluator::clearStats()
{
    for(size_t i = 0; i < Evaluator::N_ProcTimer; ++i)
    {
        itsProcTimers[i].reset();
    }
}

void Evaluator::dumpStats(ostream &out) const
{
    out << "Processing statistics: " << endl;
    for(size_t i = 0; i < Evaluator::N_ProcTimer; ++i)
    {
        const double elapsed = itsProcTimers[i].getElapsed();
        const unsigned long long count = itsProcTimers[i].getCount();

        out << "TIMER s " << Evaluator::theirProcTimerNames[i] << " total "
            << elapsed << " count " << count << " avg " << elapsed / count
            << endl;
    }
}

} //# namespace BBS
} //# namespace LOFAR
