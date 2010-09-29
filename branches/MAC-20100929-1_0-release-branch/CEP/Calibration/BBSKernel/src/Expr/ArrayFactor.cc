//# ArrayFactor.cc: Compute the array factor of a LOFAR station.
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
#include <BBSKernel/Expr/ArrayFactor.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

ArrayFactor::ArrayFactor(const Expr<Vector<2> >::ConstPtr &direction,
    const Expr<Vector<2> >::ConstPtr &reference,
    const AntennaSelection &selection, double referenceFreq)
    :   BasicBinaryExpr<Vector<2>, Vector<2>, JonesMatrix>(direction,
            reference),
        itsSelection(selection),
        itsReferenceFreq(referenceFreq)
{
}

const JonesMatrix::View ArrayFactor::evaluateImpl(const Grid &grid,
    const Vector<2>::View &direction, const Vector<2>::View &reference) const
{
    const size_t nElement = itsSelection.size();
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();

    // Check preconditions.
    ASSERT(!direction(0).isComplex() && !direction(1).isComplex());
    ASSERT(direction(0).nx() == 1
        && static_cast<size_t>(direction(0).ny()) == nTime);
    ASSERT(direction(1).nx() == 1
        && static_cast<size_t>(direction(1).ny()) == nTime);
    ASSERT(!reference(0).isComplex() && !reference(1).isComplex());
    ASSERT(reference(0).nx() == 1
        && static_cast<size_t>(reference(0).ny()) == nTime);
    ASSERT(reference(1).nx() == 1
        && static_cast<size_t>(reference(1).ny()) == nTime);

    // Compute propagation vectors. Note: The propagation vectors are computed
    // in the local East, North, Up coordinate system, to allow subsequent
    // computation of the inner product with the element (dipole) position
    // vectors (which are expressed in the same system). Azimuth is defined
    // North over East.
    Matrix sin_az = sin(direction(0));
    Matrix cos_az = cos(direction(0));
    Matrix sin_el = sin(direction(1));
    Matrix cos_el = cos(direction(1));

    Matrix k[3];
    k[0] = -cos_el * sin_az;
    k[1] = -cos_el * cos_az;
    k[2] = -sin_el;

    Matrix sin_az0 = sin(reference(0));
    Matrix cos_az0 = cos(reference(0));
    Matrix sin_el0 = sin(reference(1));
    Matrix cos_el0 = cos(reference(1));

    Matrix k0[3];
    k0[0] = -cos_el0 * sin_az0;
    k0[1] = -cos_el0 * cos_az0;
    k0[2] = -sin_el0;

    // Compute angular reference frequency.
    const double omega0 = C::_2pi * itsReferenceFreq;

    // Allocate result (initialized at 0+0i).
    Matrix arrayFactor(makedcomplex(0.0, 0.0), nFreq, nTime);
    for(size_t i = 0; i < nElement; ++i)
    {
        // Compute the delay for a plane wave approaching from the direction of
        // interest with respect to the phase center of element i.
        Matrix delay = (k[0] * itsSelection(i, 0) + k[1] * itsSelection(i, 1)
            + k[2] * itsSelection(i, 2)) / C::c;

        // Compute the delay for a plane wave approaching from the phase
        // reference direction with respect to the phase center of element i.
        Matrix delay0 = (k0[0] * itsSelection(i, 0) + k0[1] * itsSelection(i, 1)
            + k0[2] * itsSelection(i, 2)) / C::c;

        DBGASSERT(delay.nx() == 1 && static_cast<size_t>(delay.ny()) == nTime);
        DBGASSERT(delay0.nx() == 1
            && static_cast<size_t>(delay0.ny()) == nTime);

        double *p_re, *p_im;
        arrayFactor.dcomplexStorage(p_re, p_im);
        const double *p_delay = delay.doubleStorage();
        const double *p_delay0 = delay0.doubleStorage();

        for(size_t t = 0; t < nTime; ++t)
        {
            const double delay_t = *p_delay++;
            const double shift0 = omega0 * (*p_delay0++);

            for(size_t f = 0; f < nFreq; ++f)
            {
                const double shift = shift0 - C::_2pi * grid[FREQ]->center(f)
                    * delay_t;

                *p_re += std::cos(shift);
                *p_im += std::sin(shift);
                ++p_re;
                ++p_im;
            }
        }
    }

    // Normalize.
    arrayFactor /= nElement;

    JonesMatrix::View result;
    result.assign(0, 0, arrayFactor);
    result.assign(0, 1, Matrix(makedcomplex(0.0, 0.0)));
    result.assign(1, 0, Matrix(makedcomplex(0.0, 0.0)));
    result.assign(1, 1, arrayFactor);
    return result;
}

} //# namespace BBS
} //# namespace LOFAR
