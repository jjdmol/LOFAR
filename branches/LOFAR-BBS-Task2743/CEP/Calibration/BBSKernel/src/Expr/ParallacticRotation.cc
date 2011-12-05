//# ParallacticRotation.cc: Jones matrix that relates the (X,Y)-frame used to
//# express polarization on the sky (according to the IAU definition) to the
//# topocentric (theta,phi)-frame. Both are Cartesian frames defined on the
//# tangent plane, with +X towards the North, +Y towards the East, +theta away
//# from the (pseudo) zenith, and +phi East over North around the (pseudo)
//# zenith.
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
#include <BBSKernel/Expr/ParallacticRotation.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

ParallacticRotation::ParallacticRotation(
    const Expr<Vector<3> >::ConstPtr &direction,
    const AntennaField::ConstPtr &field)
    :   BasicUnaryExpr<Vector<3>, JonesMatrix>(direction),
        itsField(field)
{
}

const JonesMatrix::View ParallacticRotation::evaluateImpl(const Grid&,
    const Vector<3>::View &direction) const
{
    // Check preconditions.
    ASSERTSTR(!direction(0).isComplex() && !direction(1).isComplex()
        && !direction(2).isComplex(), "Source directions should be real"
        " valued.");

    // Compute the cross product of the NCP and the target direction. This
    // yields a vector tangent to the celestial sphere at the target direction,
    // pointing towards the East (the direction of +Y in the IAU definition,
    // or positive right ascension).
    Matrix v1x = -direction(1);
    Matrix v1y = direction(0);
    Matrix v1z = Matrix(0.0);

    Matrix v1norm = sqrt(sqr(v1x) + sqr(v1y) + sqr(v1z));
    v1x = v1x / v1norm;
    v1y = v1y / v1norm;
    v1z = v1z / v1norm;

    // Compute the cross product of the antenna field normal (R) and the target
    // direction. This yields a vector tangent to the topocentric spherical
    // coordinate system at the target direction, pointing towards the direction
    // of positive phi (which runs East over North around the pseudo zenith).
    const Vector3 &r = itsField->axis(AntennaField::R);
    Matrix v2x = direction(2) * r[1] - direction(1) * r[2];
    Matrix v2y = direction(0) * r[2] - direction(2) * r[0];
    Matrix v2z = direction(1) * r[0] - direction(0) * r[1];

    Matrix v2norm = sqrt(sqr(v2x) + sqr(v2y) + sqr(v2z));
    v2x = v2x / v2norm;
    v2y = v2y / v2norm;
    v2z = v2z / v2norm;

    // Compute the cosine and sine of the parallactic angle, i.e. the angle
    // between v1 and v2, both tangent to a latitude circle of their respective
    // spherical coordinate systems.
    Matrix cospar = v1x * v2x + v1y * v2y + v1z * v2z;
    Matrix sinpar = (v1y * v2z - v1z * v2y) * direction(0)
        + (v1z * v2x - v1x * v2z) * direction(1)
        + (v1x * v2y - v1y * v2x) * direction(2);

    // The input coordinate system is a right handed system with its third axis
    // along the direction of propagation (IAU +Z). The output coordinate system
    // is right handed as well, but its third axis points in the direction of
    // arrival (i.e. exactly opposite).
    //
    // Because the electromagnetic field is always perpendicular to the
    // direction of propagation, we only need to relate the (X, Y) axes of the
    // input system to the corresponding (theta, phi) axes of the output system.
    //
    // To this end, we first rotate the input system around its third axis to
    // align the Y axis with the phi axis. The X and theta axis are parallel
    // after this rotation, but point in opposite directions. To align the X
    // axis with the theta axis, we flip it.
    //
    // The Jones matrix to align the Y axis with the phi axis when these are
    // separated by an angle phi (measured counter-clockwise around the
    // direction of propagation, looking towards the origin), is given by:
    //
    // [ cos(phi)  sin(phi)]
    // [-sin(phi)  cos(phi)]
    //
    // Here, cos(phi) and sin(phi) can be computed directly, without having to
    // compute phi first (see the computation of cospar and sinpar above).
    //
    // Now, sinpar as computed above is opposite to sin(phi), because the
    // direction used in the computation is the direction of arrival instead
    // of the direction of propagation. Therefore, the sign of sinpar needs to
    // be reversed. Furthermore, as explained above, the X axis has to be
    // flipped to align with the theta axis. The Jones matrix returned from this
    // function is therefore given by:
    //
    // [-cospar  sinpar]
    // [ sinpar  cospar]
    //
    // This is an improper rotation, or rotoreflection.

//    LOG_DEBUG_STR("COS: " << cospar);
//    LOG_DEBUG_STR("SIN: " << sinpar);

    return JonesMatrix::View(-cospar, sinpar, sinpar, cospar);
}

} //# namespace BBS
} //# namespace LOFAR
