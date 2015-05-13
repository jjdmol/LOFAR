//# MatrixInverseMMSE.cc: Robust matrix inverse suggested by Sarod Yatawatta.
//#
//# Copyright (C) 2012
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

#include <BBSKernel/Expr/MatrixInverseMMSE.h>

namespace LOFAR
{
namespace BBS
{

MatrixInverseMMSE::MatrixInverseMMSE(const Expr<JonesMatrix>::ConstPtr &expr,
    double sigma)
    :   BasicUnaryExpr<JonesMatrix, JonesMatrix>(expr),
        itsSigma(sigma)
{
}

const JonesMatrix::View MatrixInverseMMSE::evaluateImpl(const Grid&,
    const JonesMatrix::View &arg0) const
{
    JonesMatrix::View result;

    // Add the variance of the nuisance term to the elements on the diagonal.
    const double variance = itsSigma * itsSigma;
    Matrix diag0 = arg0(0, 0) + variance;
    Matrix diag1 = arg0(1, 1) + variance;

    // Compute inverse in the usual way.
    Matrix invDet(1.0 / (diag0 * diag1 - arg0(0, 1) * arg0(1, 0)));
    result.assign(0, 0, diag1 * invDet);
    result.assign(0, 1, arg0(0, 1) * -invDet);
    result.assign(1, 0, arg0(1, 0) * -invDet);
    result.assign(1, 1, diag0 * invDet);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
