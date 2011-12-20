//# AntennaFieldAzEl.cc: Compute azimuth and elevation in radians relevative to
//# the antenna field coordinate system (P, Q, R). The input direction as well
//# as the positive coordinate axes are assumed to be unit vectors expressed in
//# ITRF. Zero azimuth corresponds to the positive Q axis and positive azimuth
//# runs from the positive Q axis to the positive P axis. Elevation is the angle
//# the direction makes with the (P, Q) plane.
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
#include <BBSKernel/Expr/AntennaFieldAzEl.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

AntennaFieldAzEl::AntennaFieldAzEl(const Expr<Vector<3> >::ConstPtr &direction,
    const AntennaField::ConstPtr &field)
    :   BasicUnaryExpr<Vector<3>, Vector<2> >(direction),
        itsField(field)
{
}

const Vector<2>::View AntennaFieldAzEl::evaluateImpl(const Grid&,
    const Vector<3>::View &direction) const
{
    // Check preconditions.
    ASSERTSTR(!direction(0).isComplex() && !direction(1).isComplex()
        && !direction(2).isComplex(), "Source directions should be real"
        " valued.");

    const Vector3 &p = itsField->axis(AntennaField::P);
    const Vector3 &q = itsField->axis(AntennaField::Q);
    const Vector3 &r = itsField->axis(AntennaField::R);

    // Compute the P and Q coordinates of the direction vector by projecting
    // onto the positive P and Q axis.
    Matrix projectionP = direction(0) * p[0] + direction(1) * p[1]
        + direction(2) * p[2];
    Matrix projectionQ = direction(0) * q[0] + direction(1) * q[1]
        + direction(2) * q[2];

    // Compute the inner product between the antenna field normal (R) and the
    // direction vector to get the sine of the elevation (cosine of the zenith
    // angle).
    Matrix sinEl = direction(0) * r[0] + direction(1) * r[1]
        + direction(2) * r[2];

    // Compute azimuth and elevation. Zero azimuth corresponds to the positive Q
    // axis and positive azimuth runs from the positive Q axis to the positive P
    // axis. Elevation is computed by taking the arcsine of the angle computed
    // earlier.
    Vector<2>::View azel;
    azel.assign(0, atan2(projectionP, projectionQ));
    azel.assign(1, asin(sinEl));
    return azel;
}

} //# namespace BBS
} //# namespace LOFAR
