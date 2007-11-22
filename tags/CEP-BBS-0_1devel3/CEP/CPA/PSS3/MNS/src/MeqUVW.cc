//# MeqUVW.h: Class to calculate the UVW coordinates
//#
//# Copyright (C) 2002
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

#include <MNS/MeqUVW.h>
#include <MNS/MeqRequest.h>
#include <casa/Quanta/MVBaseline.h>
#include <casa/Arrays/Matrix.h>

// Calculate the uvw coordinates.
//   u = x*sin(HA) + y*cos(HA)
//   v = -x*sin(dec)*cos(HA) + y*sin(dec)*sin*ha) + z*cos(dec)
//   w = x*sin(dec)*cos(HA) - y*cos(dec)*sin(HA) + z*sin(dec)
//      HA = RA - t;
//      t = tcen + dt, thus HA = RA-tcen - dt;
//      sin(HA) = sin(RA-tcen) * cos(dt) - cos(RA-tcen) * sin(dt);
//      cos(HA) = cos(RA-tcen) * cos(dt) - sin(RA-tcen) * sin(dt);
// Because dt is in a small interval around 0, sin(t) and cos(t) can
// be approximated by a simple polynomial (1-x*x/2 and x-x*x*x/6).

MeqUVW::MeqUVW (double ra, double dec)
: itsRa          (ra),
  itsDec         (dec),
  itsSinDec      (sin(dec)),
  itsCosDec      (cos(dec)),
  itsLastRequest (InitMeqRequestId)
{}

void MeqUVW::calcUVW (const MeqRequest& request, const MVBaseline& baseline)
{
  // Calculate variables dependent on domain only once.
  if (request.getId() != itsLastRequest) {
    const MeqDomain& domain = request.domain();
    double tcen = domain.cenX();
    double racen = itsRa - tcen;
    itsSinRac = sin(racen);
    itsCosRac = cos(racen);
    const MeqMatrix& uval = itsU.getValue();
    if (uval.nx() != request.domain().nx()) {
      Matrix<double> tmp(request.domain().nx(), 1);
      itsU.setValue (tmp);
      itsV.setValue (tmp);
      itsW.setValue (tmp);
    }
  }

  const Vector<double> xyz = baseline.getValue();
  double x = xyz(0);
  double y = xyz(1);
  double z = xyz(2);
  double u1 = x*itsSinRac + y*itsCosRac;
  double u2 = y*itsSinRac - x*itsCosRac;
  double v1 = y*itsSinDec*itsSinRac - x*itsSinDec*itsCosRac;
  double v2 = y*itsSinDec*itsCosRac + x*itsSinDec*itsSinRac;
  double v3 = z*itsCosDec;
  double w1 = x*itsSinDec*itsSinRac + y*itsCosDec*itsCosRac;
  double w2 = x*itsSinDec*itsCosRac - y*itsCosDec*itsSinRac;
  double w3 = z*itsSinDec;
  double dt = request.domain().startX();
  double* u = itsU.getValue().doubleStorage();
  double* v = itsV.getValue().doubleStorage();
  double* w = itsW.getValue().doubleStorage();
  for (int i=0; i<request.domain().nx(); i++) {
    double sindt = dt - dt*dt*dt/6;
    double cosdt = 1 - dt*dt/2;
    u[i] = u1*cosdt + u2*sindt;
    v[i] = v1*cosdt - v2*sindt + v3;
    w[i] = w1*sindt + w2*cosdt + w3;
  } 
}
