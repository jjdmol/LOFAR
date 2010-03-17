//# FaradayRotation.cc: Ionospheric Faraday rotation.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/Expr/FaradayRotation.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

FaradayRotation::FaradayRotation(const Expr<Scalar>::ConstPtr &rm)
    :   BasicUnaryExpr<Scalar, JonesMatrix>(rm)
{
}

const JonesMatrix::View FaradayRotation::evaluateImpl(const Grid &grid,
    const Scalar::View &rm) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // TODO: Create special case for scalar rotation measure using scalars
    // instead of full 2-D matrices.
    Matrix lambdaSqr;
    double *origin = lambdaSqr.setDoubleFormat(nFreq, nTime);

    // Have to create a full 2-D matrix because MeqMatrix does not support 1-D
    // arrays.
    for(unsigned int f = 0; f < nFreq; ++f)
    {
        double *sample = origin + f;

        // Precompute lambda squared for the current frequency point.
        const double value = std::pow(C::c / grid[FREQ]->center(f), 2);
        for(unsigned int t = 0; t < nTime; ++t)
        {
            *sample = value;
            sample += nFreq;
        }
    }

    Matrix chi = rm() * lambdaSqr;
    Matrix cosChi = cos(chi);
    Matrix sinChi = sin(chi);

    JonesMatrix::View result;
    result.assign(0, 0, cosChi);
    result.assign(0, 1, -sinChi);
    result.assign(1, 0, sinChi);
    result.assign(1, 1, cosChi);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
