//# ElevationCut.cc: Trivial beam model that is equal to 1.0 above the elevation
//# cut-off and 0.0 otherwise.
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
#include <BBSKernel/Expr/ElevationCut.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

ElevationCut::ElevationCut(const Expr<Vector<2> >::ConstPtr &azel,
    double threshold)
    :   BasicUnaryExpr<Vector<2>, JonesMatrix>(azel),
        itsThreshold(casa::C::pi * threshold / 180.0)
{
}

const JonesMatrix::View ElevationCut::evaluateImpl(const Grid &grid,
    const Vector<2>::View &azel) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(static_cast<size_t>(azel(0).nelements()) == nTime);
    ASSERT(static_cast<size_t>(azel(1).nelements()) == nTime);

    if(!azel.bound(1))
    {
        return JonesMatrix::View();
    }

    // Allocate result (uninitialized).
    Matrix mask(dcomplex(), nFreq, nTime, false);

    // Get pointers to input and output data.
    const double *in = azel(1).doubleStorage();
    double *out_re = 0, *out_im = 0;
    mask.dcomplexStorage(out_re, out_im);

    // Compute elevation mask.
    for(size_t t = 0; t < nTime; ++t, ++in)
    {
        const double value = (*in >= itsThreshold ? 1.0 : 0.0);
        for(size_t f = 0; f < nFreq; ++f, ++out_re, ++out_im)
        {
            *out_re = value;
            *out_im = 0.0;
        }
    }

    Matrix zero(makedcomplex(0.0, 0.0));
    return JonesMatrix::View(mask, zero, zero, mask);
}

} //# namespace BBS
} //# namespace LOFAR
