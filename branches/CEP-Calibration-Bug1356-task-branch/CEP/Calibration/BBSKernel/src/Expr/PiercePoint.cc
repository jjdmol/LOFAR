//# PiercePoint.cc: Pierce point for a direction (az, el) on the sky.
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

namespace LOFAR
{
namespace BBS
{

const double PiercePoint::theirIonosphereHeight = 400000.0;

PiercePoint::PiercePoint(const casa::MPosition &position,
    const Expr<Vector<2> >::ConstPtr &direction)
    :   BasicUnaryExpr<Vector<2>, Vector<4> >(direction),
        itsPosition(casa::MPosition::Convert(position, casa::MPosition::ITRF)())
{
    // Get antenna position (ITRF).
    const casa::MVPosition &ant_pos = itsPosition.getValue();
    const double x_ant = ant_pos(0);
    const double y_ant = ant_pos(1);
    const double z_ant = ant_pos(2);

    // Get lon, lat, height.
    casa::Vector<casa::Double> ang;
    casa::MPosition::Convert loc2(itsPosition, casa::MPosition::WGS84);
    casa::MPosition locwgs84(loc2());
    ang = locwgs84.getAngle().getValue();
    itsLon = ang(0);
    itsLat = ang(1);
    itsHeight = locwgs84.getValue().getLength().getValue();

    // Use height above surface to calculate earth_radius at position of the
    // station.
    double inproduct_xyz = std::sqrt(x_ant * x_ant + y_ant * y_ant
        + z_ant * z_ant);
    itsEarthRadius = inproduct_xyz - itsHeight;
}

const Vector<4>::View PiercePoint::evaluateImpl(const Request &request,
    const Vector<2>::View &azel) const
{
    const size_t nTime = request[TIME]->size();

    // Get long lat needed for rotation.
    double sinlon = std::sin(itsLon);
    double coslon = std::cos(itsLon);
    double sinlat = std::sin(itsLat);
    double coslat = std::cos(itsLat);

    // Get station position in ITRF.
    const casa::MVPosition &ant_pos = itsPosition.getValue();
    const double x_ant = ant_pos(0);
    const double y_ant = ant_pos(1);
    const double z_ant = ant_pos(2);

    // Allocate space for the result.
    // TODO: This is a hack! The Matrix class does not support 1xN or Nx1
    // "matrices".
    Matrix out_x, out_y, out_z, out_alpha;
    double *x = out_x.setDoubleFormat(1, nTime);
    double *y = out_y.setDoubleFormat(1, nTime);
    double *z = out_z.setDoubleFormat(1, nTime);
    // Use lon,lat,height instead?? or convert at MeqMIM??
    double *alpha = out_alpha.setDoubleFormat(1, nTime);

    for(size_t i = 0; i < nTime; ++i)
    {
        const double az = azel(0).getDouble(0, i);
        const double el = azel(1).getDouble(0, i);

        // Calculate alpha, this is the angle of the line of sight with the phase
        // screen (i.e. alpha' in the document).
        *alpha = std::asin(std::cos(el) * (itsEarthRadius + itsHeight)
            / (itsEarthRadius + theirIonosphereHeight));

        // Direction in local coordinates.
        double sinaz=std::sin(az);
        double cosaz=std::cos(az);
        double sinel=std::sin(el);
        double cosel=std::cos(el);
        double dir_vector[3] = {sinaz * cosel, cosaz * cosel, sinel};

        // Now convert.
        double xdir = -sinlon * dir_vector[0] - sinlat * coslon * dir_vector[1]
            + coslat * coslon * dir_vector[2];
        double ydir = coslon * dir_vector[0] - sinlat * sinlon * dir_vector[1]
            + coslat * sinlon * dir_vector[2];
        double zdir = coslat * dir_vector[1] + sinlat * dir_vector[2];

        double scale = (itsEarthRadius + theirIonosphereHeight)
            * std::sin(0.5 * casa::C::pi - el - *alpha) / cosel;

        *x = x_ant + xdir * scale;
        *y = y_ant + ydir * scale;
        *z = z_ant + zdir * scale;

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
