//# AntennaElementLBA.cc: Model of an idealized LOFAR LBA dual dipole antenna
//# element.
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
#include <BBSKernel/Expr/AntennaElementLBA.h>
#include <ElementResponse/ElementResponse.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

AntennaElementLBA::AntennaElementLBA(const Expr<Vector<2> >::ConstPtr &target)
    :   BasicUnaryExpr<Vector<2>, JonesMatrix>(target)
{
}

const JonesMatrix::View AntennaElementLBA::evaluateImpl(const Grid &grid,
    const Vector<2>::View &target) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(static_cast<size_t>(target(0).nelements()) == nTime);
    ASSERT(static_cast<size_t>(target(1).nelements()) == nTime);

    // Get pointers to input and output data.
    const double *theta = target(0).doubleStorage();
    const double *phi = target(1).doubleStorage();

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

    // Evaluate antenna model.
    dcomplex J[2][2];
    for(size_t i = 0; i < nTime; ++i)
    {
        for(size_t j = 0; j < nFreq; ++j)
        {
            // The positive X dipole direction is SW of the reference
            // orientation, which translates to a phi coordinate of 5/4*pi in
            // the topocentric spherical coordinate system. The phi coordinate
            // is corrected for this offset before evaluating the antenna model.
            element_response_lba(grid[FREQ]->center(j), theta[i],
                phi[i] - 5.0 * casa::C::pi_4, J);

            *E00_re++ = real(J[0][0]);
            *E00_im++ = imag(J[0][0]);
            *E01_re++ = real(J[0][1]);
            *E01_im++ = imag(J[0][1]);
            *E10_re++ = real(J[1][0]);
            *E10_im++ = imag(J[1][0]);
            *E11_re++ = real(J[1][1]);
            *E11_im++ = imag(J[1][1]);
        }
    }

    return JonesMatrix::View(E00, E01, E10, E11);
}

} //# namespace BBS
} //# namespace LOFAR
