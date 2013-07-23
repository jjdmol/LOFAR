//# PiercePoint.cc: Pierce point for a direction (az, el) on the sky. This node
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

#include <lofar_config.h>

#include <BBSKernel/Expr/PiercePoint.h>

#include <casa/BasicSL/Constants.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasConvert.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>

#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{

PiercePoint::PiercePoint(const casa::MPosition &position,
    const Expr<Vector<2> >::ConstPtr &direction,
    const Expr<Scalar>::ConstPtr &height)
    :   BasicBinaryExpr<Vector<2>, Scalar, Vector<4> >(direction, height),
        itsPosition(casa::MPosition::Convert(position,
            casa::MPosition::ITRF)())
{
    // Get geodetic longitude, geodetic lattitude, and height above the
    // ellipsoid (WGS84).
    casa::MPosition mPositionWGS84 =
        casa::MPosition::Convert(itsPosition, casa::MPosition::WGS84)();
    const casa::MVPosition &mvPositionWGS84 = mPositionWGS84.getValue();
    itsLon = mvPositionWGS84.getLong();
    itsLat = mvPositionWGS84.getLat();

    // Compute the distance from the center of gravity of the earth to the
    // station position.
    itsPositionRadius = itsPosition.getValue().getLength().getValue();

    // Use height above the ellipsoid to compute the earth radius at the
    // position of the station.
    //
    // TODO: You cannot just subtract the height above the ellipsoid from the
    // length of the ITRF position vector and expect to get the earth radius at
    // the station position. Assuming "earth radius" means distance from the
    // center of mass of the earth to the position on the ellipsoid specified by
    // itsLat, itsLon, then still this is incorrect because the position vector
    // is generally not parallel to the ellipsoidal normal vector at the
    // position on the ellipsoid at itsLat, itsLon (along which the height above
    // the ellipsoid is measured).
    //
    // TODO: This earth radius is specific to the station position, yet it is
    // also used when computing the length of the pierce point vector (which
    // intersects the earth's surface at a different position, where the earth
    // radius may be different!).
    itsEarthRadius = itsPositionRadius - mvPositionWGS84.getLength().getValue();

//    LOG_DEBUG_STR("Antenna: Longitude: " << (itsLon * 180.0) / casa::C::pi
//        << " deg, Lattitude: " << (itsLat * 180.0) / casa::C::pi
//        << " deg, Height: " << itsHeight << " m, Radius: " << radius
//        << " m, Earth radius: " << itsEarthRadius << " m");
//    LOG_DEBUG_STR("Antenna: Longitude: " << itsLon
//        << " rad, Lattitude: " << itsLat
//        << " rad, Height: " << itsHeight << " m, Radius: " << radius
//        << " m, Earth radius: " << itsEarthRadius << " m");
}

const Vector<4>::View PiercePoint::evaluateImpl(const Grid &grid,
    const Vector<2>::View &azel, const Scalar::View &height) const
{
    const size_t nTime = grid[TIME]->size();

    // Allocate space for the result.
    // TODO: This is a hack! The Matrix class does not support 1xN or Nx1
    // "matrices".
    Matrix out_x, out_y, out_z, out_alpha;
    double *x = out_x.setDoubleFormat(1, nTime);
    double *y = out_y.setDoubleFormat(1, nTime);
    double *z = out_z.setDoubleFormat(1, nTime);
    double *alpha = out_alpha.setDoubleFormat(1, nTime);

    // Get station position in ITRF coordinates.
    const casa::MVPosition &mPosition = itsPosition.getValue();
    double stationX = mPosition(0);
    double stationY = mPosition(1);
    double stationZ = mPosition(2);

//    LOG_DEBUG_STR("Geocentric lattitude: " << std::atan2(stationZ, std::sqrt(stationX * stationX + stationY * stationY)));

    // Precompute rotation matrix to transform from local ENU (East, North, Up)
    // coordinates to ITRF coordinates (see e.g.
    // http://en.wikipedia.org/wiki/Geodetic_system).
    //
    // TODO: Use measures to compute a direction vector in ITRF directly,
    // instead of going from (RA, Dec) to (azimuth, elevation), to ENU to ITRF.
    double sinlon = std::sin(itsLon);
    double coslon = std::cos(itsLon);
    double sinlat = std::sin(itsLat);
    double coslat = std::cos(itsLat);
    double R[3][3] = {{-sinlon, -sinlat * coslon, coslat * coslon},
                      { coslon, -sinlat * sinlon, coslat * sinlon},
                      {    0.0,           coslat,          sinlat}};

//    {
//        // A first stab at using the measures to compute a direction vector
//        // in ITRF coordinates directly. Needs more work.
//        casa::Quantity qEpoch(grid[TIME]->center(0), "s");
//        casa::MEpoch mEpoch(qEpoch, casa::MEpoch::UTC);

//        // Create and initialize a frame.
//        casa::MeasFrame frame;
//        frame.set(itsPosition);
//        frame.set(mEpoch);

//        // Create conversion engine.
//        casa::MDirection mDirection(casa::MVDirection(4.33961, 1.09537),
//            casa::MDirection::Ref(casa::MDirection::J2000));

//        casa::MDirection::Convert converter =
//            casa::MDirection::Convert(mDirection,
//                casa::MDirection::Ref(casa::MDirection::ITRF, frame));

//        LOG_DEBUG_STR("Convertor: " << converter);

//        // Compute XYZ direction vector.
//        casa::MVDirection mvXYZ(converter().getValue());
//        const casa::Vector<casa::Double> xyz = mvXYZ.getValue();
//        LOG_DEBUG_STR("XYZ direction (from measures): " << xyz(0) << " "
//            << xyz(1) << " " << xyz(2));
//    }

    ASSERT(azel(0).isArray() && azel(0).nx() == 1);
    ASSERT(azel(1).isArray() && azel(1).nx() == 1);
    ASSERT(!height().isArray());

    // TODO: itsEarthRadius is valid only for the station position, which is
    // generally not equal to the position where the pierce point vector
    // intersects the ellipsoid. The question is how the height of the
    // ionospheric layer should be defined. A layer with a fixed distance from
    // the earth center of mass is probably easiest.
    double ionosphereRadius = itsEarthRadius + height().getDouble();

    for(size_t i = 0; i < nTime; ++i)
    {
        double az = azel(0).getDouble(0, i);
        double el = azel(1).getDouble(0, i);

        // Calculate alpha, this is the angle of the line of sight with the
        // normal of the ionospheric layer at the pierce point location (i.e.
        // alpha' (alpha prime) in the memo).
        *alpha =
            std::asin(std::cos(el) * (itsPositionRadius / ionosphereRadius));

        // Compute direction vector in local ENU (East, North, Up) coordinates.
        double cosel = std::cos(el);
        double dx = std::sin(az) * cosel;
        double dy = std::cos(az) * cosel;
        double dz = std::sin(el);

        // Transform the direction vector from the local ENU frame to the ITRF
        // frame.
        double dxr = R[0][0] * dx + R[0][1] * dy + R[0][2] * dz;
        double dyr = R[1][0] * dx + R[1][1] * dy + R[1][2] * dz;
        double dzr = R[2][0] * dx + R[2][1] * dy + R[2][2] * dz;

//        if(i == 0)
//        {
//            LOG_DEBUG_STR("XYZ direction (from az, el): " << dxr << " " << dyr
//                << " " << dzr);
//        }

        // Compute the distance from the station to the pierce point, i.e.
        // equation 6 in the memo. The equation below is equivalent to that in
        // the memo, but it is expressed in elevation instead of zenith angle.
        double radius = ionosphereRadius * std::cos(el + *alpha) / cosel;

        // Compute the pierce point location in ITRF coordinates.
        *x = stationX + radius * dxr;
        *y = stationY + radius * dyr;
        *z = stationZ + radius * dzr;

        ++x; ++y; ++z; ++alpha;
    }

    // Create result.
    Vector<4>::View result;
    result.assign(0, out_x);
    result.assign(1, out_y);
    result.assign(2, out_z);
    result.assign(3, out_alpha);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
