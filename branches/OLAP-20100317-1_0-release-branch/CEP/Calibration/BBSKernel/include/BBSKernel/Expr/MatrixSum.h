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

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class MatrixSum: public Expr<JonesMatrix>
{
public:
    typedef shared_ptr<MatrixSum>       Ptr;
    typedef shared_ptr<const MatrixSum> ConstPtr;

    virtual ~MatrixSum();
    void connect(const Expr<JonesMatrix>::ConstPtr &expr);

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int i) const;
    virtual const JonesMatrix evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

private:
    void merge(const ValueSet &in, ValueSet &out) const;

    vector<Expr<JonesMatrix>::ConstPtr> itsExpr;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
