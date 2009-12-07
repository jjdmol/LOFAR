//# Mul.h: Multiplication.
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

#ifndef LOFAR_BBSKERNEL_EXPR_MUL_H
#define LOFAR_BBSKERNEL_EXPR_MUL_H

// \file
// Multiplication

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class Mul: public BasicBinaryExpr<Scalar, JonesMatrix, JonesMatrix>
{
public:
    typedef shared_ptr<Mul>         Ptr;
    typedef shared_ptr<const Mul>   ConstPtr;

    Mul(const Expr<Scalar>::ConstPtr &lhs,
        const Expr<JonesMatrix>::ConstPtr &rhs);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Scalar::View &lhs, const JonesMatrix::View &rhs) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
