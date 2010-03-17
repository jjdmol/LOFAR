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

GaussianCoherence::GaussianCoherence(const Expr<Vector<4> >::ConstPtr stokes,
    const Expr<Vector<2> >::ConstPtr dimensions,
    const Expr<Scalar>::ConstPtr orientation,
    const Expr<Vector<3> >::ConstPtr &uvwA,
    const Expr<Vector<3> >::ConstPtr &uvwB)
    :   BasicExpr5<Vector<4>, Vector<2>, Scalar, Vector<3>, Vector<3>,
            JonesMatrix>(stokes, dimensions, orientation, uvwA, uvwB)
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

    // Compute baseline uv-coordinates in meters (1D in time).
    Matrix u = uvwB(0) - uvwA(0);
    Matrix v = uvwB(1) - uvwA(1);

    // Convert orientation from degrees to radians and convert to positive
    // North over East, East is +90 degrees.
    // TODO: Can probably optimize by changing the rotation matrix instead.
    Matrix phi = orientation() * (-casa::C::pi/180.0) + casa::C::pi_2;
    Matrix cosPhi(cos(phi));
    Matrix sinPhi(sin(phi));

    // Rotate (u, v) by the orientation and scale with the major and minor axis
    // lengths (FWHM). Take care of the conversion of FWHM to sigma.
    const double arcsec2rad = (casa::C::pi / 3600.0) / 180.0;
    const double fwhm2sigma = 1.0 / (2.0 * std::sqrt(2.0 * std::log(2.0)));

    Matrix uPrime =
        dimensions(0) * (arcsec2rad * fwhm2sigma) * (u * cosPhi - v * sinPhi);

    Matrix vPrime =
        dimensions(1) * (arcsec2rad * fwhm2sigma) * (u * sinPhi + v * cosPhi);

    // Compute uPrime^2 + vPrime^2 and pre-multiply with -2.0 * PI^2 / C^2.
    Matrix uvPrime =
        (-2.0 * casa::C::pi * casa::C::pi) * (sqr(uPrime) + sqr(vPrime));

    // Compute spatial coherence (2D).
    const unsigned int nFreq = grid[FREQ]->size();
    const unsigned int nTime = grid[TIME]->size();
    ASSERT(uvPrime.nx() == 1
        && static_cast<unsigned int>(uvPrime.ny()) == nTime);

    Matrix coherence;
    double *it = coherence.setDoubleFormat(nFreq, nTime);
    for(unsigned int ts = 0; ts < nTime; ++ts)
    {
        const double uv = uvPrime.getDouble(0, ts);
        for(unsigned int ch = 0; ch < nFreq; ++ch)
        {
            const double lambda_inv = grid[FREQ]->center(ch) / casa::C::c;
            *it++ = std::exp(lambda_inv * lambda_inv * uv);
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
