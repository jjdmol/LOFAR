//# TFRequest.cc: The request for an evaluation of an expression
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

#include <MNS/TFRequest.h>

TFRequest::TFRequest (const TFDomain& domain)
: itsDomain (domain)
{
  fillPowers();
}

TFRequest::TFRequest (const TFDomain& domain, int nrSpid)
: itsDomain (domain),
  itsNspids (nrSpid)
{
  fillPowers();
}

void TFRequest::fillPowers()
{
  int nx = itsDomain.nx();
  int ny = itsDomain.ny();
  itsPowers.resize (nx*ny);
  double powers[100];
  // Calculate all cross-terms for 2-dim polynomials with max. order 9.
  // Thus x, x^2 .. x^n, xy, x^2y .. x^ny, xy^2 .. x^ny^2 .. x^ny^n
  // Do that for all values in the domain.
  int inx = 0;
  double y = itsDomain.startY();
  for (int iy=0; iy<ny; iy++) {
    double x = itsDomain.startX();
    for (int ix=0; ix<nx; ix++) {
      double* powptr = powers;
      double yv = 1;
      for (int j=0; j<9; j++) {
	double xv = 1;
	for (int i=0; i<9; i++) {
	  *powptr++ = xv*yv;
	  xv *= x;
	}
	yv *= y;
      }
      itsPowers[inx++] = MnsMatrix(powers, 10, 10);
      x += itsDomain.stepX();
    }
    y += itsDomain.stepY();
  }
}
