//# ITRFDirection.cc: Compute ITRF direction vector for a J2000 direction (RA,
//# DEC) on the sky.
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

#include <BBSKernel/Expr/ITRFDirection.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/Quantum.h>

namespace LOFAR
{
namespace BBS
{

ITRFDirection::ITRFDirection(const casa::MPosition &position,
    const Expr<Vector<2> >::ConstPtr &direction)
    :   BasicUnaryExpr<Vector<2>, Vector<3> >(direction),
        itsPosition(casa::MPosition::Convert(position, casa::MPosition::ITRF)())
{
}

const Vector<3>::View ITRFDirection::evaluateImpl(const Grid &grid,
    const Vector<2>::View &direction) const
{
    // Check preconditions.
    ASSERTSTR(!direction(0).isArray() && !direction(1).isArray(), "Variable"
        " source directions not supported (yet).");
    ASSERTSTR(!direction(0).isComplex() && !direction(1).isComplex(), "Source"
        " directions should be real valued.");

    // Initialize reference frame.
    casa::Quantum<casa::Double> qEpoch(0.0, "s");
    casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);
    casa::MeasFrame mFrame(mEpoch, itsPosition);

    // Create conversion engine.
    casa::MDirection mDirection(casa::MVDirection(direction(0).getDouble(),
        direction(1).getDouble()),
        casa::MDirection::Ref(casa::MDirection::J2000));

    // Setup coordinate transformation engine.
    casa::MDirection::Convert convertor(mDirection,
        casa::MDirection::Ref(casa::MDirection::ITRF, mFrame));

    // Allocate space for the result.
    // TODO: This is a hack! The Matrix class does not support 1xN or Nx1
    // "matrices".
    const size_t nTime = grid[TIME]->size();

    Matrix x, y, z;
    double *x_p = x.setDoubleFormat(1, nTime);
    double *y_p = y.setDoubleFormat(1, nTime);
    double *z_p = z.setDoubleFormat(1, nTime);

    for(size_t i = 0; i < nTime; ++i)
    {
        // Update reference frame.
        qEpoch.setValue(grid[TIME]->center(i));
        mEpoch.set(qEpoch);
        mFrame.set(mEpoch);

        // Compute ITRF direction vector.
        casa::MVDirection mvITRF(convertor().getValue());
        *x_p++ = mvITRF(0);
        *y_p++ = mvITRF(1);
        *z_p++ = mvITRF(2);
    }

    Vector<3>::View result;
    result.assign(0, x);
    result.assign(1, y);
    result.assign(2, z);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
