//# TileArrayFactor.cc: Compute the array factor of a LOFAR HBA tile.
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
#include <BBSKernel/Expr/TileArrayFactor.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

TileArrayFactor::TileArrayFactor(const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Vector<3> >::ConstPtr &reference,
    const AntennaField::ConstPtr &field,
    bool conjugate)
    :   BasicBinaryExpr<Vector<3>, Vector<3>, Scalar>(direction, reference),
        itsField(field),
        itsConjugateFlag(conjugate)
{
}

const Scalar::View TileArrayFactor::evaluateImpl(const Grid &grid,
    const Vector<3>::View &direction, const Vector<3>::View &reference) const
{
    const size_t nFreq = grid[FREQ]->size();
    const size_t nTime = grid[TIME]->size();
    const size_t nElement = itsField->nTileElement();

    // Check preconditions.
    ASSERT(!direction(0).isComplex() && direction(0).nx() == 1
        && static_cast<size_t>(direction(0).ny()) == nTime);
    ASSERT(!direction(1).isComplex() && direction(1).nx() == 1
        && static_cast<size_t>(direction(1).ny()) == nTime);
    ASSERT(!direction(2).isComplex() && direction(2).nx() == 1
        && static_cast<size_t>(direction(2).ny()) == nTime);

    ASSERT(!reference(0).isComplex() && reference(0).nx() == 1
        && static_cast<size_t>(reference(0).ny()) == nTime);
    ASSERT(!reference(1).isComplex() && reference(1).nx() == 1
        && static_cast<size_t>(reference(1).ny()) == nTime);
    ASSERT(!reference(2).isComplex() && reference(2).nx() == 1
        && static_cast<size_t>(reference(2).ny()) == nTime);

    // Instead of computing a phase shift for the pointing direction and a phase
    // shift for the direction of interest and then computing the difference,
    // compute the resultant phase shift in one go. Here we make use of the
    // relation a . b + a . c = a . (b + c). The sign of k is related to the
    // sign of the phase shift.
    Matrix k[3];
    k[0] = direction(0) - reference(0);
    k[1] = direction(1) - reference(1);
    k[2] = direction(2) - reference(2);

    // Allocate result (initialized at 0+0i).
    Matrix arrayFactor(makedcomplex(0.0, 0.0), nFreq, nTime);
    for(size_t i = 0; i < nElement; ++i)
    {
        // Compute the effective delay for a plane wave approaching from the
        // direction of interest with respect to the phase center of element i
        // when beam forming in the reference direction using time delays.
        const Vector3 &offset = itsField->tileElement(i);
        Matrix delay = (k[0] * offset[0] + k[1] * offset[1] + k[2] * offset[2])
            / C::c;

        // Turn the delay into a phase shift.
        double *p_re, *p_im;
        arrayFactor.dcomplexStorage(p_re, p_im);
        const double *p_delay = delay.doubleStorage();
        for(size_t t = 0; t < nTime; ++t)
        {
            const double delay_t = *p_delay++;
            for(size_t f = 0; f < nFreq; ++f)
            {
                const double shift = C::_2pi * grid[FREQ]->center(f) * delay_t;
                *p_re += std::cos(shift);
                *p_im += std::sin(shift);
                ++p_re;
                ++p_im;
            }
        }
    }

    // Normalize.
    if(nElement > 0)
    {
        arrayFactor /= nElement;
    }

    // Conjugate if required.
    if(itsConjugateFlag)
    {
        arrayFactor = conj(arrayFactor);
    }

    return Scalar::View(arrayFactor);
}

} //# namespace BBS
} //# namespace LOFAR
