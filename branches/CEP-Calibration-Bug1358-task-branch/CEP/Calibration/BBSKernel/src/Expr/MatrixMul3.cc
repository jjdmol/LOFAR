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

MatrixMul3::MatrixMul3()
    :   ExprStatic<MatrixMul3::N_Inputs>()
{
}    

ValueSet::ConstPtr MatrixMul3::evaluateImpl(const Request &request,
    const ValueSet::ConstPtr (&inputs)[MatrixMul3::N_Inputs]) const
{
    ValueSet::ConstPtr left = inputs[LEFT];
    ValueSet::ConstPtr mid = inputs[MID];
    ValueSet::ConstPtr right = inputs[RIGHT];
    
    // TODO: generalize this.
    ASSERT(left->rank() == 2 && left->shape(0) == 2 && left->shape(1) == 2);
    ASSERT(mid->rank() == 2 && mid->shape(0) == 2 && mid->shape(1) == 2);
    ASSERT(right->rank() == 2 && right->shape(0) == 2 && right->shape(1) == 2);

    // Compute LEFT * MID.
    Matrix tmp00(left->value(0, 0) * mid->value(0, 0)
        + left->value(0, 1) * mid->value(1, 0));
    Matrix tmp01(left->value(0, 0) * mid->value(0, 1)
        + left->value(0, 1) * mid->value(1, 1));
    Matrix tmp10(left->value(1, 0) * mid->value(0, 0)
        + left->value(1, 1) * mid->value(1, 0));
    Matrix tmp11(left->value(1, 0) * mid->value(0, 1)
        + left->value(1, 1) * mid->value(1, 1));

    // Compute (LEFT * MID) * RIGHT^H.
    ValueSet::Ptr result(new ValueSet(2, 2));
    result->assign(0, 0, Matrix(tmp00 * conj(right->value(0, 0))
         + tmp01 * conj(right->value(0, 1))));
    result->assign(0, 1, Matrix(tmp00 * conj(right->value(1, 0))
         + tmp01 * conj(right->value(1, 1))));
    result->assign(1, 0, Matrix(tmp10 * conj(right->value(0, 0))
         + tmp11 * conj(right->value(0, 1))));
    result->assign(1, 1, Matrix(tmp10 * conj(right->value(1, 0))
         + tmp11 * conj(right->value(1, 1))));

    return result;
}

} // namespace BBS
} // namespace LOFAR
