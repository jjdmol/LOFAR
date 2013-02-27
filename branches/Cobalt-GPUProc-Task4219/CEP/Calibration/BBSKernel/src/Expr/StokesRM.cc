//# StokesRM.cc: Stokes vector with Q and U parameterized by polarized fraction,
//# polarization angle, and rotation measure.
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
#include <BBSKernel/Expr/StokesRM.h>
#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

StokesRM::StokesRM(const Expr<Scalar>::ConstPtr &stokesI,
    const Expr<Scalar>::ConstPtr &stokesV,
    const Expr<Scalar>::ConstPtr &polFraction,
    const Expr<Scalar>::ConstPtr &polAngle0,
    const Expr<Scalar>::ConstPtr &rm)
    :   BasicExpr5<Scalar, Scalar, Scalar, Scalar, Scalar, Vector<4> >(stokesI,
            stokesV, polFraction, polAngle0, rm)
{
}

const Vector<4>::View StokesRM::evaluateImpl(const Grid &grid,
    const Scalar::View &stokesI,
    const Scalar::View &stokesV,
    const Scalar::View &polFraction,
    const Scalar::View &polAngle0,
    const Scalar::View &rm) const
{
    Vector<4>::View stokes;

    if(stokesI.bound())
    {
        stokes.assign(0, stokesI());
    }

    if(stokesI.bound() || polFraction.bound() || polAngle0.bound()
        || rm.bound())
    {
        const size_t nFreq = grid[FREQ]->size();
        const size_t nTime = grid[TIME]->size();

        Matrix lambdaSqr;
        double *origin = lambdaSqr.setDoubleFormat(nFreq, nTime);

        // Have to create a full 2-D matrix because MeqMatrix does not support
        // 1-D arrays.
        for(unsigned int f = 0; f < nFreq; ++f)
        {
            // Precompute lambda squared for the current frequency point.
            const double lambda = C::c / grid[FREQ]->center(f);
            const double lambda2 = lambda * lambda;

            double *sample = origin + f;
            for(unsigned int t = 0; t < nTime; ++t)
            {
                *sample = lambda2;
                sample += nFreq;
            }
        }

        Matrix chi = 2.0 * (polAngle0() + rm() * lambdaSqr);
        Matrix stokesQU = stokesI() * polFraction();
        Matrix stokesQ = stokesQU * cos(chi);
        Matrix stokesU = stokesQU * sin(chi);

        stokes.assign(1, stokesQ);
        stokes.assign(2, stokesU);
    }

    if(stokesV.bound())
    {
        stokes.assign(3, stokesV());
    }

    return stokes;
}

} //# namespace BBS
} //# namespace LOFAR
