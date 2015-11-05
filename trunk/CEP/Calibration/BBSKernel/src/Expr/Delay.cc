//# Delay.cc: Phase shift corresponding to a (differential) delay in seconds.
//#
//# Copyright (C) 2010
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
#include <BBSKernel/Expr/Delay.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

Delay::Delay(const Expr<Scalar>::ConstPtr &delay)
    :   BasicUnaryExpr<Scalar, Scalar>(delay)
{
}

const Scalar::View Delay::evaluateImpl(const Grid &grid,
    const Scalar::View &delay) const
{
    // Create a 2-D Matrix that contains the frequency for each sample. (We
    // _must_ expand to a 2-D buffer because 1-D buffers are not supported).
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    Matrix freq;
    double *it = freq.setDoubleFormat(nFreq, nTime);
    for(unsigned int t = 0; t < nTime; ++t)
    {
        for(unsigned int f = 0; f < nFreq; ++f)
        {
            *it++ = grid[FREQ]->center(f);
        }
    }

    Matrix phase = freq * (delay() * casa::C::_2pi);
    Matrix shift = tocomplex(cos(phase), sin(phase));

    return Scalar::View(shift);
}

} //# namespace BBS
} //# namespace LOFAR
