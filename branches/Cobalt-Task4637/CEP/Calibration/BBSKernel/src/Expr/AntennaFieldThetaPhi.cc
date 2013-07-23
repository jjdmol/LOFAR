//# AntennaFieldThetaPhi.cc: Compute topocentric (local) theta and phi spherical
//# coordinates (in radians) relative to the Cartesian antenna field coordinate
//# system (PQR), for a given target direction. The target direction is assumed
//# to be an ITRF unit vector in the direction of arrival. The positive
//# coordinate axes are assumed to be ITRF unit vectors. Zero phi corresponds to
//# the positive P axis, and positive phi runs from the positive P axis towards
//# the positive Q axis (roughly East over North). Theta or zenith angle is the
//# angle the target direction makes with the positive R axis.
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
#include <BBSKernel/Expr/AntennaFieldThetaPhi.h>
#include <casa/BasicSL/Constants.h>

namespace LOFAR
{
namespace BBS
{

AntennaFieldThetaPhi::AntennaFieldThetaPhi(const Expr<Vector<3> >::ConstPtr &direction,
    const AntennaField::ConstPtr &field)
    :   BasicUnaryExpr<Vector<3>, Vector<2> >(direction),
        itsField(field)
{
}

const Vector<2>::View AntennaFieldThetaPhi::evaluateImpl(const Grid&,
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
    // direction vector to get the cosine of the zenith angle (sine of the
    // elevation).
    Matrix projectionR = direction(0) * r[0] + direction(1) * r[1]
        + direction(2) * r[2];

    // Compute theta and phi. Zero phi corresponds to the positive P axis and
    // positive phi runs from the positive P axis to the positive Q axis.
    Vector<2>::View result;
    result.assign(0, acos(projectionR));
    result.assign(1, atan2(projectionQ, projectionP));

//    LOG_DEBUG_STR("EP: [" << p[0] << ", " << p[1] << ", " << p[2] << "]");
//    LOG_DEBUG_STR("EQ: [" << q[0] << ", " << q[1] << ", " << q[2] << "]");
//    LOG_DEBUG_STR("ER: [" << r[0] << ", " << r[1] << ", " << r[2] << "]");
//    LOG_DEBUG_STR("X: " << direction(0));
//    LOG_DEBUG_STR("Y: " << direction(1));
//    LOG_DEBUG_STR("Z: " << direction(2));
//    LOG_DEBUG_STR("THETA: " << result(0));
//    LOG_DEBUG_STR("PHI: " << result(1));

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
