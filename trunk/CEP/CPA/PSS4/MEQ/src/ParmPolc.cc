//# ParmPolc.cc: Polynomial coefficients
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

#include <MEQ/ParmPolc.h>
#include <MEQ/Request.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>


namespace MEQ {

ParmPolc::ParmPolc (const string& name)
: Parm (name)
{}

ParmPolc::~ParmPolc()
{}

int ParmPolc::initDomain (const Domain&, int spidIndex)
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


int ParmPolc::getResult (Result::Ref& result, const Request& request)
{
  int spidIndex=0;
  initDomain (request.cells().domain(), spidIndex);
  // Create result object and attach to datarecord in the Node object.
  Result* resp = new Result();
  result <<= resp;
  wstate()[AidResult] <<= static_cast<DataRecord*>(resp);
  // A single polc can be evaluated immediately.
  if (itsPolcs.size() == 1) {
    itsPolcs[0].getResult (result, request);
    return 0;
  }
  // Get the domain, etc.
  const Cells& cells = request.cells();
  const Domain& domain = cells.domain();
  int ndFreq = cells.nfreq();
  int ndTime = cells.ntime();
  double* datar = 0;
  double stepFreq = cells.stepFreq();
  double halfStepFreq = stepFreq * .5;
  double firstMidFreq = domain.startFreq() + halfStepFreq;
  double lastMidFreq  = firstMidFreq + (ndFreq-1) * stepFreq;
  double firstMidTime = cells.time(0);
  double lastMidTime  = cells.time(ndTime-1);
  Result& res = result.dewr();
  res.setReal (ndFreq, ndTime);
  // Iterate over all polynomials.
  // Evaluate one if its domain overlaps the request domain.
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    Polc& polc = itsPolcs[i];
    const Domain& polDom = polc.domain();
    if (firstMidFreq < polDom.endFreq() && lastMidFreq > polDom.startFreq()
    &&  firstMidTime < polDom.endTime() && lastMidTime > polDom.startTime()) {
      // Determine which part of the request domain is covered by the
      // polynomial.
      int stFreq = 0;
      if (firstMidFreq < polDom.startFreq()) {
	stFreq = 1 + int((polDom.startFreq() - firstMidFreq) / stepFreq);
      }
      int nrFreq = ndFreq - stFreq;
      if (lastMidFreq > polDom.endFreq()) {
	int remFreq = 1 + int((lastMidFreq - polDom.endFreq()) / stepFreq);
	nrFreq -= remFreq;
      }
      int stTime = 0;
      while (cells.time(stTime) < polDom.startTime()) {
	stTime++;
      }
      int lastTime = ndTime-1;
      while (cells.time(lastTime) > polDom.endTime()) {
	lastTime--;
      }
      int nrTime = lastTime - stTime + 1;
      // If the overlap is full, only this polynomial needs to be evaluated.
      if (stFreq == 0  &&  nrFreq == ndFreq
      &&  stTime == 0  &&  nrTime == ndTime) {
	polc.getResult (result, request);
	return 0;
      }
      // Form the domain and request for the overlapping part
      // and evaluate the polynomial.
      double startFreq = domain.startFreq() + stFreq*stepFreq;
      double startTime = cells.time(stTime) - cells.stepTime(stTime) / 2;
      double endTime   = cells.time(lastTime) + cells.stepTime(lastTime) / 2;
      Domain partDom(startFreq, startFreq + nrFreq*stepFreq,
		     startTime, endTime);
      LoVec_double partStartTime(nrTime);
      LoVec_double partEndTime(nrTime);
      for (int j=0; j<nrTime; j++) {
	partStartTime(j) = cells.time(stTime+j) - cells.stepTime(stTime+j);
	partEndTime(j)   = cells.time(stTime+j) + cells.stepTime(stTime+j);
      }
      Cells partCells (partDom, nrFreq, partStartTime, partEndTime);
      Request partReq(partCells);
      Result partRes;
      Result::Ref partResRef (partRes, DMI::WRITE||DMI::EXTERNAL);
      polc.getResult (partResRef, partReq);
      // Create the result matrix if it is the first Time.
      // Now it is initialized with zeroes (to see possible errors).
      // In the future the outcommnented statement can be used
      // which saves the initialization Time. It requires that the
      // request domain is entirely covered by the polcs.
      if (datar == 0) {
	LoMat_double& mat = res.setReal (ndFreq, ndTime);
	datar = mat.data();
      }
      // Move the values to the correct place in the output result.
      // Note that in principle a polynomial could be a single coefficient
      // in which case it returns a single value.
      const double* from = partRes.getValue().realStorage();
      double* to = datar + stFreq + stTime*ndFreq;
      if (partRes.getValue().nelements() == 1) {
	for (int iTime=0; iTime<nrTime; iTime++) {
	  for (int iFreq=0; iFreq<nrFreq; iFreq++) {
	    to[iFreq] = *from;
	  }
	  to += ndFreq;
	}
      } else {
	for (int iTime=0; iTime<nrTime; iTime++) {
	  for (int iFreq=0; iFreq<nrFreq; iFreq++) {
	    to[iFreq] = *from++;
	  }
	  to += ndFreq;
	}
      }
    }
  }
  return 0;
}

void ParmPolc::getInitial (Vells& values) const
{
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    itsPolcs[i].getInitial (values);
  }
}

void ParmPolc::getCurrentValue(Vells& value, bool denormalize) const
{
  Assert(1 == itsPolcs.size());
  itsPolcs[0].getCurrentValue(value, denormalize);
}

void ParmPolc::update (const Vells& value)
{
  for (unsigned int i=0; i<itsPolcs.size(); i++) {
    itsPolcs[i].update (value);
  }
}

void ParmPolc::save()
{}

} //namespace MEQ
