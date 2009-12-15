//# JonesInvert.cc: The inverse of a Jones matrix expression.
//#
//# Copyright (C) 2005
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

#include <BBSKernel/Expr/JonesInvert.h>

// Inverse of a 2x2 matrix:
//
//        (a b)                      ( d -b)
// If A = (   ) then inverse(A)   =  (     ) / (ad-bc)
//        (c d)                      (-c  a)
//
// Note that:
//            (conj(a) conj(c))
//  conj(A) = (               )
//            (conj(b) conj(d))
//
// so  inverse(conj(A)) = conj(inverse(A))
//
// Also note that conj(AB) = conj(B)conj(A)

namespace LOFAR
{
namespace BBS
{

JonesInvert::JonesInvert(const Expr<JonesMatrix>::ConstPtr &expr)
    :   BasicUnaryExpr<JonesMatrix, JonesMatrix>(expr)
{
}

const JonesMatrix::View JonesInvert::evaluateImpl(const Grid &grid,
    const JonesMatrix::View &arg0) const
{
    JonesMatrix::View result;

    Matrix invDet(1.0 / (arg0(0, 0) * arg0(1, 1) - arg0(0, 1) * arg0(1, 0)));
    result.assign(0, 0, arg0(1, 1) * invDet);
    result.assign(0, 1, arg0(0, 1) * -invDet);
    result.assign(1, 0, arg0(1, 0) * -invDet);
    result.assign(1, 1, arg0(0, 0) * invDet);

    return result;
}

} // namespace BBS
} // namespace LOFAR
