//# GaussianCoherence.cc: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
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

#include <BBSKernel/Expr/GaussianCoherence.h>

#include <Common/lofar_complex.h>
#include <Common/lofar_math.h>
#include <Common/LofarLogger.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{
using LOFAR::exp;
using LOFAR::conj;

GaussianCoherence::GaussianCoherence(const GaussianSource::ConstPtr &source,
    const Expr<Vector<3> >::ConstPtr &uvwA,
    const Expr<Vector<3> >::ConstPtr &uvwB)
    :   BasicExpr5<Vector<4>, Vector<2>, Scalar, Vector<3>, Vector<3>,
            JonesMatrix>(source->getStokesVector(), source->getDimensions(),
            source->getOrientation(), uvwA, uvwB)
{
}

const JonesMatrix::View GaussianCoherence::evaluateImpl(const Grid &grid,
    const Vector<4>::View &stokes, const Vector<2>::View &dimensions,
    const Scalar::View &orientation, const Vector<3>::View &uvwA,
    const Vector<3>::View &uvwB) const
{
    // Assume dimensions and orientation are frequency and time independent.
    ASSERT(!dimensions(0).isArray());
    ASSERT(!dimensions(1).isArray());
    ASSERT(!orientation().isArray());

    JonesMatrix::View result;

    // Compute baseline uv-coordinates (1D in time).
    Matrix uBaseline = uvwB(0) - uvwA(0);
    Matrix vBaseline = uvwB(1) - uvwA(1);

    // Compute dot product of a rotated, scaled uv-vector with itself (1D in
    // time) and pre-multiply with 2.0 * PI^2 / C^2.
    Matrix cosPhi(cos(orientation()));
    Matrix sinPhi(sin(orientation()));
    Matrix uvTransformed = (2.0 * casa::C::pi * casa::C::pi)
        / (casa::C::c * casa::C::c)
        * (sqr(dimensions(0) * (uBaseline * cosPhi - vBaseline * sinPhi))
        + sqr(dimensions(1) * (uBaseline * sinPhi + vBaseline * cosPhi)));

    // Compute spatial coherence (2D).
    const unsigned int nChannels = grid[FREQ]->size();
    const unsigned int nTimeslots = grid[TIME]->size();

    Matrix coherence;
    double *it = coherence.setDoubleFormat(nChannels, nTimeslots);

    for(unsigned int ts = 0; ts < nTimeslots; ++ts)
    {
        const double uv = uvTransformed.getDouble(0, ts);
        for(unsigned int ch = 0; ch < nChannels; ++ch)
        {
            const double freq = grid[FREQ]->center(ch);
            *it++ = exp(-(freq * freq * uv));
        }
    }

    const bool bound = dimensions.bound(0) || dimensions.bound(1)
        || orientation.bound();

    if(bound || stokes.bound(0) || stokes.bound(1))
    {
        result.assign(0, 0, coherence * 0.5 * (stokes(0) + stokes(1)));
        result.assign(1, 1, coherence * 0.5 * (stokes(0) - stokes(1)));
    }

    if(bound || stokes.bound(2) || stokes.bound(3))
    {
        Matrix uv = coherence * 0.5 * tocomplex(stokes(2), stokes(3));
        result.assign(0, 1, uv);
        result.assign(1, 0, conj(uv));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
