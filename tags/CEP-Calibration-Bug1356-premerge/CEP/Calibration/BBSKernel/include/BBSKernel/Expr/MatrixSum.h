//# MatrixSum.h: Compute the (element-wise) sum of a collection of Jones
//# matrices.
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

#ifndef LOFAR_BBSKERNEL_EXPR_MATRIXSUM_H
#define LOFAR_BBSKERNEL_EXPR_MATRIXSUM_H

// \file
// Compute the (element-wise) sum of a collection of Jones matrices.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprResult.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class MatrixSum: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<MatrixSum>       Ptr;
    typedef shared_ptr<const MatrixSum> ConstPtr;

    ~MatrixSum()
    {
        for(size_t i = 0; i < itsExpr.size(); ++i)
        {
            disconnect(itsExpr[i]);
        }
    }

    void connect(const Expr<JonesMatrix>::ConstPtr &expr)
    {
        ExprBase::connect(expr);
        itsExpr.push_back(expr);
    }

protected:
    virtual unsigned int nArguments() const
    {
        return itsExpr.size();
    }

    virtual ExprBase::ConstPtr argument(unsigned int i) const
    {
        ASSERT(i < itsExpr.size());
        ASSERT(itsExpr[i]);
        return itsExpr[i];
    }

private:
    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache)
        const;

    void merge(const ValueSet &in, ValueSet &out) const;

    vector<Expr<JonesMatrix>::ConstPtr> itsExpr;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
