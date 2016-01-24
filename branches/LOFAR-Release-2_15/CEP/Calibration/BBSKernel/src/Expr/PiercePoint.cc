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

const double earth_ellipsoid_a = 6378137.0;
const double earth_ellipsoid_a2 = earth_ellipsoid_a*earth_ellipsoid_a;
const double earth_ellipsoid_b = 6356752.3142;
const double earth_ellipsoid_b2 = earth_ellipsoid_b*earth_ellipsoid_b;
const double earth_ellipsoid_e2 = (earth_ellipsoid_a2 - earth_ellipsoid_b2) / earth_ellipsoid_a2;


PiercePoint::PiercePoint(const casa::MPosition &position,
    const Expr<Vector<3> >::ConstPtr &direction,
    const Expr<Scalar>::ConstPtr &height)
    :   BasicBinaryExpr<Vector<3>, Scalar, Vector<4> >(direction, height),
        itsPosition(casa::MPosition::Convert(position,
            casa::MPosition::ITRF)())
{
}

const Vector<4>::View PiercePoint::evaluateImpl(const Grid &grid,
    const Vector<3>::View &direction, const Scalar::View &height) const
{
    const size_t nTime = grid[TIME]->size();
    const size_t nFreq = grid[FREQ]->size();

    // Allocate space for the result.
    // TODO: This is a hack! The Matrix class does not support 1xN or Nx1
    // "matrices".
    Matrix out_x, out_y, out_z, out_airmass;
    double *out_x_ptr = out_x.setDoubleFormat(nFreq, nTime);
    double *out_y_ptr = out_y.setDoubleFormat(nFreq, nTime);
    double *out_z_ptr = out_z.setDoubleFormat(nFreq, nTime);
    double *out_airmass_ptr = out_airmass.setDoubleFormat(nFreq, nTime);
    double pp_x, pp_y, pp_z, pp_airmass;


    ASSERT(!height().isArray());
    double h = height().getDouble();

    // Get station position in ITRF coordinates.
    const casa::MVPosition &mPosition = itsPosition.getValue();
    double stationX = mPosition(0);
    double stationY = mPosition(1);
    double stationZ = mPosition(2);

    const double ion_ellipsoid_a = earth_ellipsoid_a + h;
    const double ion_ellipsoid_a2_inv = 1.0 / (ion_ellipsoid_a * ion_ellipsoid_a);
    const double ion_ellipsoid_b = earth_ellipsoid_b + h;
    const double ion_ellipsoid_b2_inv = 1.0 / (ion_ellipsoid_b * ion_ellipsoid_b);

    double x = stationX/ion_ellipsoid_a;
    double y = stationY/ion_ellipsoid_a;
    double z = stationZ/ion_ellipsoid_b;
    double c = x*x + y*y + z*z - 1.0;

    for(size_t i = 0; i < nTime; ++i)
    {
      double directionX = direction(0).getDouble(0, i);
      double directionY = direction(1).getDouble(0, i);
      double directionZ = direction(2).getDouble(0, i);

      double dx = directionX / ion_ellipsoid_a;
      double dy = directionY / ion_ellipsoid_a;
      double dz = directionZ / ion_ellipsoid_b;

      double a = dx*dx + dy*dy + dz*dz;
      double b = x*dx + y*dy  + z*dz;
      double alpha = (-b + std::sqrt(b*b - a*c))/a;

      pp_x = stationX + alpha*directionX;
      pp_y = stationY + alpha*directionY;
      pp_z = stationZ + alpha*directionZ;
      double normal_x = pp_x * ion_ellipsoid_a2_inv;
      double normal_y = pp_y * ion_ellipsoid_a2_inv;
      double normal_z = pp_z * ion_ellipsoid_b2_inv;
      double norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
      double norm_normal = std::sqrt(norm_normal2);
      double sin_lat2 = normal_z*normal_z / norm_normal2;

      double g = 1.0 - earth_ellipsoid_e2*sin_lat2;
      double sqrt_g = std::sqrt(g);

      double M = earth_ellipsoid_b2 / ( earth_ellipsoid_a * g * sqrt_g );
      double N = earth_ellipsoid_a / sqrt_g;

      double local_ion_ellipsoid_e2 = (M-N) / ((M+h)*sin_lat2 - N - h);
      double local_ion_ellipsoid_a = (N+h) * std::sqrt(1.0 - local_ion_ellipsoid_e2*sin_lat2);
      double local_ion_ellipsoid_b = local_ion_ellipsoid_a*std::sqrt(1.0 - local_ion_ellipsoid_e2);

      double z_offset = ((1.0-earth_ellipsoid_e2)*N + h - (1.0-local_ion_ellipsoid_e2)*(N+h)) * std::sqrt(sin_lat2);

      double x1 = stationX/local_ion_ellipsoid_a;
      double y1 = stationY/local_ion_ellipsoid_a;
      double z1 = (stationZ-z_offset)/local_ion_ellipsoid_b;
      double c1 = x1*x1 + y1*y1 + z1*z1 - 1.0;

      dx = directionX / local_ion_ellipsoid_a;
      dy = directionY / local_ion_ellipsoid_a;
      dz = directionZ / local_ion_ellipsoid_b;
      a = dx*dx + dy*dy + dz*dz;
      b = x1*dx + y1*dy  + z1*dz;
      alpha = (-b + std::sqrt(b*b - a*c1))/a;

      pp_x = stationX + alpha*directionX;
      pp_y = stationY + alpha*directionY;
      pp_z = stationZ + alpha*directionZ;

      normal_x = pp_x / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
      normal_y = pp_y / (local_ion_ellipsoid_a * local_ion_ellipsoid_a);
      normal_z = (pp_z-z_offset) / (local_ion_ellipsoid_b * local_ion_ellipsoid_b);

      norm_normal2 = normal_x*normal_x + normal_y*normal_y + normal_z*normal_z;
      norm_normal = std::sqrt(norm_normal2);
      
      pp_airmass = norm_normal / (directionX*normal_x + directionY*normal_y + directionZ*normal_z);
      
      for(size_t j = 0; j < nFreq; ++j) {
        *(out_x_ptr++) = pp_x;
        *(out_y_ptr++) = pp_y;
        *(out_z_ptr++) = pp_z;
        *(out_airmass_ptr++) = pp_airmass;
      }
    }
    // Create result.
    Vector<4>::View result;
    result.assign(0, out_x);
    result.assign(1, out_y);
    result.assign(2, out_z);
    result.assign(3, out_airmass);

    return result;
}

} //# namespace BBS
} //# namespace LOFAR
