//# MatrixMul2.cc: Compute A * B, where A and B are JonesMatrices.
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

#include <lofar_config.h>

#include <BBSKernel/Expr/MatrixMul2.h>

namespace LOFAR
{
namespace BBS
{

MatrixMul2::MatrixMul2(const Expr<JonesMatrix>::ConstPtr &lhs,
    const Expr<JonesMatrix>::ConstPtr &rhs)
    :   BasicBinaryExpr<JonesMatrix, JonesMatrix, JonesMatrix>(lhs, rhs)
{
}

const JonesMatrix::View MatrixMul2::evaluateImpl(const Request &request,
    const JonesMatrix::View &lhs, const JonesMatrix::View &rhs) const
{
    // Determine dependencies.
    const bool boundLhsRow0 = lhs.bound(0, 0) || lhs.bound(0, 1);
    const bool boundLhsRow1 = lhs.bound(1, 0) || lhs.bound(1, 1);

    const bool boundRhsCol0 = rhs.bound(0, 0) || rhs.bound(1, 0);
    const bool boundRhsCol1 = rhs.bound(0, 1) || rhs.bound(1, 1);

    // Create the result.
    JonesMatrix::View result;

    if(boundLhsRow0 || boundRhsCol0)
    {
        result.assign(0, 0, lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0));
    }

    if(boundLhsRow0 || boundRhsCol1)
    {
        result.assign(0, 1, lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1));
    }

    if(boundLhsRow1 || boundRhsCol0)
    {
        result.assign(1, 0, lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0));
    }

    if(boundLhsRow1 || boundRhsCol1)
    {
        result.assign(1, 1, lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
