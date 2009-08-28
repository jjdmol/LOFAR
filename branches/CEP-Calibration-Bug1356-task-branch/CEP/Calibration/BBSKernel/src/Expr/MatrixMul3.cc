//# MatrixMul3.cc: Compute A * B * C^H (e.g. to apply an effect described by a
//# pair of station specific Jones matrices (A, C) to a set of visibilities (B).
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

#include <BBSKernel/Expr/MatrixMul3.h>

namespace LOFAR
{
namespace BBS
{

MatrixMul3::MatrixMul3(const Expr<JonesMatrix>::ConstPtr &left,
    const Expr<JonesMatrix>::ConstPtr &mid,
    const Expr<JonesMatrix>::ConstPtr &right)
    :   BasicTernaryExpr<JonesMatrix, JonesMatrix, JonesMatrix, JonesMatrix>
            (left, mid, right)
{
}

const JonesMatrix::View MatrixMul3::evaluateImpl(const Request &request,
    const JonesMatrix::View &left, const JonesMatrix::View &mid,
    const JonesMatrix::View &right) const
{
    // Determine dependencies.
    // TODO: Should this be cached?
    bool eval00, eval01, eval10, eval11;
    eval00 = eval01 = eval10 = eval11 = false;

    if(mid.dirty(0, 0) || mid.dirty(0, 1) || mid.dirty(1, 0)
        || mid.dirty(1, 1))
    {
        eval00 = eval01 = eval10 = eval11 = true;
    }
    else
    {
        if(left.dirty(0, 0) || left.dirty(0, 1))
        {
            eval00 = eval01 = true;
        }

        if(left.dirty(1, 0) || left.dirty(1, 1))
        {
            eval10 = eval11 = true;
        }

        if(right.dirty(0, 0) || right.dirty(0, 1))
        {
            eval00 = eval10 = true;
        }

        if(right.dirty(1, 0) || right.dirty(1, 1))
        {
            eval01 = eval11 = true;
        }
    }

    // Create the result.
    JonesMatrix::View result;

    if(eval00 || eval01)
    {
        Matrix tmp00(left(0, 0) * mid(0, 0) + left(0, 1) * mid(1, 0));
        Matrix tmp01(left(0, 0) * mid(0, 1) + left(0, 1) * mid(1, 1));

        if(eval00)
        {
            result.assign(0, 0, tmp00 * conj(right(0, 0))
                + tmp01 * conj(right(0, 1)));
        }

        if(eval01)
        {
            result.assign(0, 1, tmp00 * conj(right(1, 0))
                + tmp01 * conj(right(1, 1)));
        }
    }

    if(eval10 || eval11)
    {
        Matrix tmp10(left(1, 0) * mid(0, 0) + left(1, 1) * mid(1, 0));
        Matrix tmp11(left(1, 0) * mid(0, 1) + left(1, 1) * mid(1, 1));

        if(eval10)
        {
            result.assign(1, 0, tmp10 * conj(right(0, 0))
                + tmp11 * conj(right(0, 1)));
        }

        if(eval11)
        {
            result.assign(1, 1, tmp10 * conj(right(1, 0))
                + tmp11 * conj(right(1, 1)));
        }
    }

//    // Compute LEFT * MID.
//    Matrix tmp00(left(0, 0) * mid(0, 0) + left(0, 1) * mid(1, 0));
//    Matrix tmp01(left(0, 0) * mid(0, 1) + left(0, 1) * mid(1, 1));
//    Matrix tmp10(left(1, 0) * mid(0, 0) + left(1, 1) * mid(1, 0));
//    Matrix tmp11(left(1, 0) * mid(0, 1) + left(1, 1) * mid(1, 1));

//    // Compute (LEFT * MID) * RIGHT^H.
//    JonesMatrix::View result;
//    result.assign(0, 0, tmp00 * conj(right(0, 0)) + tmp01 * conj(right(0, 1)));
//    result.assign(0, 1, tmp00 * conj(right(1, 0)) + tmp01 * conj(right(1, 1)));
//    result.assign(1, 0, tmp10 * conj(right(0, 0)) + tmp11 * conj(right(0, 1)));
//    result.assign(1, 1, tmp10 * conj(right(1, 0)) + tmp11 * conj(right(1, 1)));

    return result;
}

} // namespace BBS
} // namespace LOFAR
