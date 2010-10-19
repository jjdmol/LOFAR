//# YatawattaDipole.cc: Dipole voltage beam based on external functions.
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

#include <BBSKernel/Expr/YatawattaDipole.h>

#include <Common/lofar_complex.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

YatawattaDipole::YatawattaDipole(const casa::Path &moduleTheta,
    const casa::Path &modulePhi, const Expr<Vector<2> >::ConstPtr &azel,
    const Expr<Scalar>::ConstPtr &orientation)
    :   BasicBinaryExpr<Vector<2>, Scalar, JonesMatrix>(azel, orientation),
        itsThetaFunction(moduleTheta, "test"),
        itsPhiFunction(modulePhi, "test")
{
    ASSERT(itsThetaFunction.nArguments() == 5);
    ASSERT(itsPhiFunction.nArguments() == 5);
}

const JonesMatrix::View YatawattaDipole::evaluateImpl(const Grid &grid,
    const Vector<2>::View &azel, const Scalar::View &orientation) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(static_cast<size_t>(azel(0).nelements()) == nTime);
    ASSERT(static_cast<size_t>(azel(1).nelements()) == nTime);
    ASSERT(static_cast<size_t>(orientation().nelements()) == 1);

    // Get pointers to input and output data.
    const double *az = azel(0).doubleStorage();
    const double *el = azel(1).doubleStorage();
    const double angle = orientation().getDouble(0, 0);

    Matrix E00, E01, E10, E11;
    double *E00_re, *E00_im;
    E00.setDCMat(nFreq, nTime);
    E00.dcomplexStorage(E00_re, E00_im);

    double *E01_re, *E01_im;
    E01.setDCMat(nFreq, nTime);
    E01.dcomplexStorage(E01_re, E01_im);

    double *E10_re, *E10_im;
    E10.setDCMat(nFreq, nTime);
    E10.dcomplexStorage(E10_re, E10_im);

    double *E11_re, *E11_im;
    E11.setDCMat(nFreq, nTime);
    E11.dcomplexStorage(E11_re, E11_im);

    //  Parameters for external functions:
    //      0: time
    //          (NOTE: ignored)
    //      1: frequency
    //      2: az
    //      3: el
    //          (NOTE: incorrectly labelled zenith angle in implementation!)
    //      4: orientation (phi0)

    // Create argument vectors for the X and Y dipole (used for calling
    // external functions).
    vector<dcomplex> xArgs(5, makedcomplex(0.0, 0.0));
    vector<dcomplex> yArgs(5, makedcomplex(0.0, 0.0));

    // TODO: Inside external function, these parameters are added to the
    // azimuth. The resulting azimuth is therefore:
    //
    // az = az + orientation (- pi / 2.0)
    //
    // Whereas it seems to me that the orientation should be subtracted
    // instead of added. It probably does not matter much, because the
    // beam pattern is symmetric with respect to azimuth.
    xArgs[4] = makedcomplex(angle, 0.0);
    yArgs[4] = makedcomplex(angle - casa::C::pi_2, 0.0);

    // Evaluate beam.
    for(size_t t = 0; t < nTime; ++t)
    {
        // TODO: Where does the -pi/4 term in azimuth come from (see
        // global_model.py in EJones_HBA)? Is this just the default dipole
        // orientation? If so, the term should be removed in favor of setting
        // a correct dipole orientation in the parameter database (such that
        // the orientation in the parameter database corresponds 1:1 with the
        // real orientation).
//        xArgs[2] = yArgs[2] = makedcomplex(az[t] - casa::C::pi_4, 0.0);
        xArgs[2] = yArgs[2] = makedcomplex(az[t], 0.0);
        xArgs[3] = yArgs[3] = makedcomplex(el[t], 0.0);

        for(size_t f = 0; f < nFreq; ++f)
        {
            // Update frequency.
            xArgs[1] = yArgs[1] = makedcomplex(grid[FREQ]->center(f), 0.0);

            // Compute dipole beam value.
            const dcomplex xTheta = itsThetaFunction(xArgs);
            const dcomplex xPhi = itsPhiFunction(xArgs);
            *E00_re++ = real(xTheta);
            *E00_im++ = imag(xTheta);
            *E01_re++ = real(xPhi);
            *E01_im++ = imag(xPhi);

            const dcomplex yTheta = itsThetaFunction(yArgs);
            const dcomplex yPhi = itsPhiFunction(yArgs);
            *E10_re++ = real(yTheta);
            *E10_im++ = imag(yTheta);
            *E11_re++ = real(yPhi);
            *E11_im++ = imag(yPhi);
        }
    }

    JonesMatrix::View result;
    result.assign(0, 0, E00);
    result.assign(0, 1, E01);
    result.assign(1, 0, E10);
    result.assign(1, 1, E11);

    return result;
}


} //# namespace BBS
} //# namespace LOFAR
