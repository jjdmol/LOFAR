//# TECU2Phase.cc: Convert from TEC units to a frequency dependent phase shift.
//#
//# Copyright (C) 2011
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
#include <BBSKernel/Expr/TECU2Phase.h>

namespace LOFAR
{
namespace BBS
{

TECU2Phase::TECU2Phase(const Expr<Scalar>::ConstPtr &tec)
    :   BasicUnaryExpr<Scalar, Scalar>(tec)
{
}

const Scalar::View TECU2Phase::evaluateImpl(const Grid &grid,
    const Scalar::View &tec) const
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

    // Phase shift is equal to -k/nu * TECU, where k is a scaling
    // constant that is equal to 1.0e16 * c * r_e, where nu denotes
    // frequency, c the speed of light, and r_e the classical electron
    // radius (which is defined as e^2 / (4 * pi * e0 * m_e * c^2),
    // where e is the elementary charge, e0 is the electric constant,
    // and m_e elektron mass).
    //
    // (See e.g. http://en.wikipedia.org/wiki/Total_electron_content)
    //
    // TODO: Find out if and how to sign of the phase shift due to the
    // ionosphere is related to the definition of the plane wave used.
    Matrix phase = (tec() * -8.44797245e9) / freq;

    return Scalar::View(tocomplex(cos(phase), sin(phase)));
}

} //# namespace BBS
} //# namespace LOFAR
