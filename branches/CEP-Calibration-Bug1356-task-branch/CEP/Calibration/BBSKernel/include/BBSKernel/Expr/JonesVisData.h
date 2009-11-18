//# JonesVisData.h: Make visibility data from an observation available to the
//# expression tree.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSKERNEL_EXPR_JONESVISDATA_H
#define LOFAR_BBSKERNEL_EXPR_JONESVISDATA_H

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/VisData.h>

// \file
// Make visibility data from an observation available to the expression tree.

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class JonesVisData: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<JonesVisData>        Ptr;
    typedef shared_ptr<const JonesVisData>  ConstPtr;

    JonesVisData(const VisData::Ptr &chunk, const baseline_t &baseline);

protected:
    virtual unsigned int nArguments() const
    {
        return 0;
    }

    virtual ExprBase::ConstPtr argument(unsigned int) const
    {
        ASSERT(false);
    }

    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache)
        const;

private:
    void copyData(double *re, double *im,
        const boost::multi_array<sample_t, 4>::const_array_view<2>::type &src)
        const;

    void copyFlags(FlagArray::iterator dest,
        const boost::multi_array<flag_t, 4>::const_array_view<2>::type &src)
        const;

    VisData::Ptr    itsChunk;
    uint            itsBaselineIndex;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
