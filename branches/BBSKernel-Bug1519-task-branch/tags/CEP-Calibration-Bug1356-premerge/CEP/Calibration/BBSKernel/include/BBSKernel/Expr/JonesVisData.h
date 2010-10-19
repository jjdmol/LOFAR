//# JonesVisData.h: Make visibility data from an observation available to the
//# expression tree.
//#
//# Copyright (C) 2007
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

// \ingroup Expr
// @{

class JonesVisData: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<JonesVisData>    Ptr;
    typedef shared_ptr<JonesVisData>    ConstPtr;

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

private:
    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache)
        const;

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
