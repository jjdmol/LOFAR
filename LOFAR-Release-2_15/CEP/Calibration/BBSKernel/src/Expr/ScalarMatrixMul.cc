//# ScalarMatrixMul.cc: Multiplication of a matrix expression by a scalar
//# expression.
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#include <BBSKernel/Expr/ScalarMatrixMul.h>
#include <BBSKernel/Expr/MatrixTmp.h>

namespace LOFAR
{
namespace BBS
{

ScalarMatrixMul::ScalarMatrixMul(const Expr<Scalar>::ConstPtr &lhs,
    const Expr<JonesMatrix>::ConstPtr &rhs)
    :   BasicBinaryExpr<Scalar, JonesMatrix, JonesMatrix>(lhs, rhs)
{
}

const JonesMatrix::View ScalarMatrixMul::evaluateImpl(const Grid&,
    const Scalar::View &lhs, const JonesMatrix::View &rhs) const
{
    JonesMatrix::View result;

    if(lhs.bound() || rhs.bound(0, 0))
    {
        result.assign(0, 0, lhs() * rhs(0, 0));
    }

    if(lhs.bound() || rhs.bound(0, 1))
    {
        result.assign(0, 1, lhs() * rhs(0, 1));
    }

    if(lhs.bound() || rhs.bound(1, 0))
    {
        result.assign(1, 0, lhs() * rhs(1, 0));
    }

    if(lhs.bound() || rhs.bound(1, 1))
    {
        result.assign(1, 1, lhs() * rhs(1, 1));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
