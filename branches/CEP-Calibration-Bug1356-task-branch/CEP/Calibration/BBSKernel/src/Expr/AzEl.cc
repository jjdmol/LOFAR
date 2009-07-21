//# AzEl.cc: Azimuth and elevation for a direction (ra, dec) on the sky.
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>

#include <BBSKernel/Expr/AzEl.h>
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

AzEl::AzEl(const casa::MPosition &position,
    const Expr<Vector<2> >::ConstPtr &direction)
    :   BasicUnaryExpr<Vector<2>, Vector<2> >(direction),
        itsPosition(casa::MPosition::Convert(position, casa::MPosition::ITRF)())
{
}

const Vector<2>::view AzEl::evaluateImpl(const Request &request,
        const Vector<2>::view &direction) const
{
    // Check preconditions.
    ASSERTSTR(!direction(0).isArray() && !direction(1).isArray(), "Variable"
        " source directions not supported (yet).");
    ASSERTSTR(!direction(0).isComplex() && !direction(1).isComplex(), "Source"
        " directions should be real valued.");

    casa::Quantum<casa::Double> qEpoch(0.0, "s");
    casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);

    // Create and initialize a frame.
    casa::MeasFrame frame;
    frame.set(itsPosition);
    frame.set(mEpoch);

    // Create conversion engine.
    casa::MDirection mDirection(casa::MVDirection(direction(0).getDouble(),
        direction(1).getDouble()),
        casa::MDirection::Ref(casa::MDirection::J2000));

    casa::MDirection::Convert converter = casa::MDirection::Convert(mDirection,
        casa::MDirection::Ref(casa::MDirection::AZEL, frame));

    // Allocate space for the result.
    // TODO: This is a hack! The Matrix class does not support 1xN or Nx1
    // "matrices".
    const size_t nTime = request[TIME]->size();

    Matrix az, el;
    double *az_p = az.setDoubleFormat(1, nTime);
    double *el_p = el.setDoubleFormat(1, nTime);

    // Compute (az, el) coordinates.
    for(size_t i = 0; i < nTime; ++i)
    {
        // Update reference frame.
        const double time = request[TIME]->center(i);

        qEpoch.setValue(time);
        mEpoch.set(qEpoch);
        frame.set(mEpoch);

        // Compute azimuth and elevation.
        casa::MVDirection mvAzel(converter().getValue());
        const casa::Vector<casa::Double> azel =
            mvAzel.getAngle("rad").getValue();
        *az_p++ = azel(0);
        *el_p++ = azel(1);
    }


    Vector<2>::view result;
    result.assign(0, az);
    result.assign(1, el);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
