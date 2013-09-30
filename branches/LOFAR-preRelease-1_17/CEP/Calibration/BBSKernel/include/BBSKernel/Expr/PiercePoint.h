//# PiercePoint.h: Pierce point for a direction (az, el) on the sky. This node
//# returns a 4-vector that contains the X, Y, Z coordinates of the pierce point
//# in ITRF coordinates in meters, and the angle between the line of sight and
//# the normal to the thin ionospheric layer at the pierce point position in
//# radians.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSKERNEL_EXPR_PIERCEPOINT_H
#define LOFAR_BBSKERNEL_EXPR_PIERCEPOINT_H

// \file
// Pierce point for a direction (az, el) on the sky. This node returns a
// 4-vector that contains the X, Y, Z coordinates of the pierce point in ITRF
// coordinates in meters, and the angle between the line of sight and the normal
// to the thin ionospheric layer at the pierce point position in radians.

#include <BBSKernel/Expr/BasicExpr.h>

#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PiercePoint: public BasicBinaryExpr<Vector<2>, Scalar, Vector<4> >
{
public:
    typedef shared_ptr<PiercePoint>         Ptr;
    typedef shared_ptr<const PiercePoint>   ConstPtr;

    PiercePoint(const casa::MPosition &position,
        const Expr<Vector<2> >::ConstPtr &azel,
        const Expr<Scalar>::ConstPtr &height);

protected:
    virtual const Vector<4>::View evaluateImpl(const Grid &grid,
        const Vector<2>::View &azel, const Scalar::View &height) const;

private:
    // Station position in ITRF coordinates.
    casa::MPosition     itsPosition;
    // Geodetic longitude (rad), geodetic latittude (rad), and height above the
    // ellipsoid (m) (WGS84) of the station.
    double              itsLon, itsLat, itsHeight;
    // Distance from the center of gravity of the earth to the station (m).
    double              itsPositionRadius;
    // Earth "radius" at the station position (m).
    double              itsEarthRadius;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
