//# ConditionNumber.h: Flag the result of an Expr<JonesMatrix> by thresholding
//# on the condition number of the Jones matrices.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_EXPR_CONDITIONNUMBER_H
#define LOFAR_BBSKERNEL_EXPR_CONDITIONNUMBER_H

// \file
// Flag the result of an Expr<JonesMatrix> by thresholding on the condition
// number of the Jones matrices.

// TODO: Refactor the flagging part into a separate class that is templated
// on a predicate? This way, the ConditionNumber only computes the condition
// number, which is much clearer.

#include <BBSKernel/Expr/Expr.h>
#include <Common/lofar_algorithm.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::min;
using LOFAR::max;

// \ingroup Expr
// @{

class ConditionNumber: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<ConditionNumber>   Ptr;
    typedef shared_ptr<ConditionNumber>   ConstPtr;

    ConditionNumber(const Expr<JonesMatrix>::ConstPtr &arg0, double threshold)
        :   itsArg0(arg0),
            itsThreshold(threshold)
    {
        connect(itsArg0);
    }

    ~ConditionNumber()
    {
        disconnect(itsArg0);
    }

    virtual const JonesMatrix evaluate(const Request &request, Cache &cache)
        const
    {
        // Copy result.
        JonesMatrix result(itsArg0->evaluate(request, cache));

        // Flag on condition number.
        const JonesMatrix::proxy J = result.value();
        if(J(0, 0).isArray() || J(1, 1).isArray())
        {
            const int nFreq = request[FREQ]->size();
            const int nTime = request[TIME]->size();

            FlagArray flags(nFreq, nTime);
            FlagArray::iterator flagIt = flags.begin();

            double maxcn = 0.0, mincn = 1e16, meancn = 0.0;

            for(int t = 0; t < nTime; ++t)
            {
                for(int f = 0; f < nFreq; ++f)
                {
                    const double norm00 = abs(J(0, 0).getDComplex(f, t));
                    const double norm11 = abs(J(1, 1).getDComplex(f, t));

                    const double cond = max(norm00, norm11)
                        / max(min(norm00, norm11), 1e-9);

                    maxcn = max(maxcn, cond);
                    mincn = min(mincn, cond);
                    meancn += cond;

                    *flagIt = (cond > itsThreshold);
                    ++flagIt;
                }
            }

            LOG_DEBUG_STR("MAX: " << maxcn << " MIN: " << mincn << " MEAN: "
                << meancn / (nFreq * nTime));

            result.setFlags(result.hasFlags() ? result.flags() | flags : flags);
        }
        else
        {
            const double norm00 = abs(J(0, 0).getDComplex());
            const double norm11 = abs(J(1, 1).getDComplex());

            const double cond = max(norm00, norm11)
                / max(min(norm00, norm11), 1e-9);

            FlagArray flags(FlagType(cond > itsThreshold));
            result.setFlags(result.hasFlags() ? result.flags() | flags : flags);
        }

        return result;
    }

protected:
    virtual unsigned int getArgumentCount() const
    {
        return 1;
    }

    virtual const ExprBase::ConstPtr getArgument(unsigned int i) const
    {
        ASSERT(i == 0);
        return itsArg0;
    }

private:
    Expr<JonesMatrix>::ConstPtr itsArg0;
    double                      itsThreshold;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
