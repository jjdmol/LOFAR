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

const JonesMatrix::view MatrixMul2::evaluateImpl(const Request &request,
    const JonesMatrix::view &lhs, const JonesMatrix::view &rhs) const
{
    // Determine dependencies.
    const bool dirtyLhsRow0 = lhs.dirty(0, 0) || lhs.dirty(0, 1);
    const bool dirtyLhsRow1 = lhs.dirty(1, 0) || lhs.dirty(1, 1);

    const bool dirtyRhsCol0 = rhs.dirty(0, 0) || rhs.dirty(1, 0);
    const bool dirtyRhsCol1 = rhs.dirty(0, 1) || rhs.dirty(1, 1);

    // Create the result.
    JonesMatrix::view result;

    if(dirtyLhsRow0 || dirtyRhsCol0)
    {
        result.assign(0, 0, lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0));
    }

    if(dirtyLhsRow0 || dirtyRhsCol1)
    {
        result.assign(0, 1, lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1));
    }

    if(dirtyLhsRow1 || dirtyRhsCol0)
    {
        result.assign(1, 0, lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0));
    }

    if(dirtyLhsRow1 || dirtyRhsCol1)
    {
        result.assign(1, 1, lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
