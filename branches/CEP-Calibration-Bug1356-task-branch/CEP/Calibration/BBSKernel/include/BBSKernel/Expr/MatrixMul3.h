//# MatrixMul3.h: Compute A * B * C^H (e.g. to apply an effect described by a
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

#ifndef LOFAR_BBSKERNEL_EXPR_MATRIXMUL3_H
#define LOFAR_BBSKERNEL_EXPR_MATRIXMUL3_H

// \file
// Compute A * B * C^H (e.g. to apply an effect described by a pair of station
// specific Jones matrices (A, C) to a set of visibilities (B).

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class MatrixMul3: public BasicTernaryExpr<JonesMatrix, JonesMatrix, JonesMatrix,
    JonesMatrix>
{
public:
    typedef shared_ptr<MatrixMul3>          Ptr;
    typedef shared_ptr<const MatrixMul3>    ConstPtr;

    MatrixMul3(const Expr<JonesMatrix>::ConstPtr &left,
        const Expr<JonesMatrix>::ConstPtr &mid,
        const Expr<JonesMatrix>::ConstPtr &right);

private:
    virtual const JonesMatrix::view evaluateImpl(const Request &request,
        const JonesMatrix::view &left, const JonesMatrix::view &mid,
        const JonesMatrix::view &right) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
