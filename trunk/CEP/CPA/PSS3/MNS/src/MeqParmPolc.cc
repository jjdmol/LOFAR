//# MeqParmPolc.cc: Polynomial coefficients
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

#include <MNS/MeqParmPolc.h>
#include <MNS/MeqRequest.h>
#include <Common/Debug.h>
#include <casa/Arrays/Matrix.h>

MeqParmPolc::MeqParmPolc (const string& name)
: MeqParm (name)
{}

MeqParmPolc::~MeqParmPolc()
{}

void MeqParmPolc::readPolcs (const MeqDomain&)
{}

int MeqParmPolc::initDomain (const MeqDomain&, int spidIndex)
{
  int nr = 0;
  if (isSolvable()) {
    // For the time being we allow only one polc if the parameter
    // has to be solved.
    AssertStr (itsPolcs.size() == 1,
	       "Multiple polcs used in the solve domain for parameter "
	       << getName());
    for (unsigned int i=0; i<itsPolcs.size(); i++) {
      int nrs = itsPolcs[i].makeSolvable (spidIndex);
      nr += nrs;
      spidIndex += nrs;
    }
  } else {
    for (unsigned int i=0; i<itsPolcs.size(); i++) {
      itsPolcs[i].clearSolvable();
    }
  }    
  return nr;
}


MeqResult MeqParmPolc::getResult (const MeqRequest& request)
{
  // A single polc can be evaluated immediately.
  if (itsPolcs.size() == 1) {
    return itsPolcs[0].getResult (request);
  }
  // Get the domain, etc.
  const MeqDomain& domain = request.domain();
  MeqResult result(0);
  int ndx = request.nx();
  int ndy = request.ny();
  double* datar = 0;
  double stepx = request.stepX();
  double stepy = request.stepY();
  double halfStepx = stepx * .5;
  double halfStepy = stepy * .5;
  double firstMidx = domain.startX() + halfStepx;
  double firstMidy = domain.startY() + halfStepy; 
  double lastMidx = firstMidx + (ndx-1) * stepx;
  double lastMidy = firstMidy + (ndy-1) * stepy;
  // Iterate over all polynomials.
  // Evaluate one if its domain overlaps the request domain.
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    MeqPolc& polc = itsPolcs[i];
    const MeqDomain& polDom = polc.domain();
    if (firstMidx < polDom.endX()  &&  lastMidx > polDom.startX()
    &&  firstMidy < polDom.endY()  &&  lastMidy > polDom.startY()) {
      // Determine which part of the request domain is covered by the
      // polynomial.
      int stx = 0;
      if (firstMidx < polDom.startX()) {
	stx = 1 + int((polDom.startX() - firstMidx) / stepx);
      }
      int nrx = ndx - stx;
      if (lastMidx > polDom.endX()) {
	int remx = 1 + int((lastMidx - polDom.endX()) / stepx);
	nrx -= remx;
      }
      int sty = 0;
      if (firstMidy < polDom.startY()) {
	sty = 1 + int((polDom.startY() - firstMidy) / stepy);
      }
      int nry = ndy - sty;
      if (lastMidy > polDom.endY()) {
	int remy = 1 + int((lastMidy - polDom.endY()) / stepy);
	nry -= remy;
      }
      // If the overlap is full, only this polynomial needs to be evaluated.
      if (stx == 0  &&  nrx == ndx  &&  sty == 0  &&  nry == ndy) {
	return polc.getResult (request);
      }
      // Form the domain and request for the overlapping part
      // and evaluate the polynomial.
      double startx = domain.startX() + stx*stepx;
      double starty = domain.startY() + sty*stepy;
      MeqDomain partDom(startx, startx + nrx*stepx,
			starty, starty + nry*stepy);
      MeqRequest partReq(partDom, nrx, nry);
      MeqResult partRes = polc.getResult (partReq);
      // Create the result matrix if it is the first time.
      // Now it is initialized with zeroes (to see possiible errors).
      // In the future the outcommnented statement can be used
      // which saves the initialization time. It requires that the
      // request domain is entirely covered by the polcs.
      if (datar == 0) {
	Matrix<double> mat(ndx, ndy);
	mat = 0;
	////	  result.setValue (MeqMatrix(double(), ndx. ndy));
	result.setValue (mat);
	datar = result.getValueRW().doubleStorage();
      }
      // Move the values to the correct place in the output result.
      // Note that in principle a polynomial could be a single coefficient
      // in which case it returns a single value.
      const double* from = partRes.getValue().doubleStorage();
      double* to = datar + stx + sty*ndx;
      if (partRes.getValue().nelements() == 1) {
	for (int iy=0; iy<nry; iy++) {
	  for (int ix=0; ix<nrx; ix++) {
	    to[ix] = *from;
	  }
	  to += ndx;
	}
      } else {
	for (int iy=0; iy<nry; iy++) {
	  for (int ix=0; ix<nrx; ix++) {
	    to[ix] = *from++;
	  }
	  to += ndx;
	}
      }
    }
  }
  return result;
}

void MeqParmPolc::getInitial (MeqMatrix& values) const
{
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    itsPolcs[i].getInitial (values);
  }
}

void MeqParmPolc::getCurrentValue(MeqMatrix& value, bool denormalize) const
{
  Assert(1 == itsPolcs.size());
  itsPolcs[0].getCurrentValue(value, denormalize);
}

void MeqParmPolc::update (const MeqMatrix& value)
{
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    itsPolcs[i].update (value);
  }
}

void MeqParmPolc::save()
{}
