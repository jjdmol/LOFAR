//# MeqExpr.cc: The base class of an expression.
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

#include <lofar_config.h>
#include <BBS3/MNS/MeqExpr.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <BBS3/MNS/MeqRequest.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>

namespace LOFAR {

MeqExprRep::~MeqExprRep()
{
  delete itsResult;
  delete itsResVec;
}

const MeqResult& MeqExprRep::calcResult (const MeqRequest& request,
					 MeqResult& result)
{
  // The value has to be calculated.
  // Do not cache if no multiple parents.
  if (itsNParents <= 1) {
    result = getResult (request);
    return result;
  }
  // Use a cache.
  // Synchronize the calculations.
// ## start critical section ##
  // Only calculate if not already calculated in another thread.
  if (itsReqId != request.getId()) {
    if (!itsResult) itsResult = new MeqResult;
    *itsResult = getResult (request);
    itsReqId = request.getId();
  }
//## end critical section ##
  return *itsResult;
}

MeqResult MeqExprRep::getResult (const MeqRequest&)
{
  THROW (LOFAR::Exception,
	 "MeqExpr::getResult not implemented in derived class");
}

const MeqResultVec& MeqExprRep::calcResultVec (const MeqRequest& request,
					       MeqResultVec& result)
{
  // The value has to be calculated.
  // Do not cache if no multiple parents.
  if (itsNParents <= 1) {
    result = getResultVec (request);
    return result;
  }
  // Use a cache.
  // Synchronize the calculations.
// ## start critical section ##
  // Only calculate if not already calculated in another thread.
  if (itsReqId != request.getId()) {
    if (!itsResVec) itsResVec = new MeqResultVec;
    *itsResVec = getResultVec (request);
    itsReqId = request.getId();
  }
//## end critical section ##
  return *itsResVec;
}

MeqResultVec MeqExprRep::getResultVec (const MeqRequest& request)
{
  MeqResultVec res(1);
  res[0] = getResult (request);
  return res;
}



MeqExpr::MeqExpr (const MeqExpr& that)
: itsRep (that.itsRep)
{
  if (itsRep != 0) {
    itsRep->link();
  }
}
MeqExpr& MeqExpr::operator= (const MeqExpr& that)
{
  if (this != &that) {
    MeqExprRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}



MeqExprToComplex::MeqExprToComplex (MeqExpr real, MeqExpr imag)
  : itsReal(real),
    itsImag(imag)
{
  itsReal.incrNParents();
  itsImag.incrNParents();
}

MeqExprToComplex::~MeqExprToComplex()
{}

MeqResult MeqExprToComplex::getResult (const MeqRequest& request)
{
  MeqResult realRes;
  MeqResult imagRes;
  const MeqResult& real = itsReal.getResultSynced (request, realRes);
  const MeqResult& imag = itsImag.getResultSynced (request, imagRes);
  MeqResult result(request.nspid());
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    if (real.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
				tocomplex(real.getPerturbedValue(spinx),
					  imag.getPerturbedValue(spinx)));
      result.setPerturbation (spinx, real.getPerturbation(spinx));
    } else if (imag.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
				tocomplex(real.getPerturbedValue(spinx),
					  imag.getPerturbedValue(spinx)));
      result.setPerturbation (spinx, imag.getPerturbation(spinx));
    }
  }
  result.setValue (tocomplex(real.getValue(), imag.getValue()));
  return result;
}




MeqExprAPToComplex::MeqExprAPToComplex (MeqExpr ampl, MeqExpr phase)
  : itsAmpl (ampl),
    itsPhase(phase)
{
  itsAmpl.incrNParents();
  itsPhase.incrNParents();
}

MeqExprAPToComplex::~MeqExprAPToComplex()
{}

MeqResult MeqExprAPToComplex::getResult (const MeqRequest& request)
{
  MeqResult amplRes;
  MeqResult phaseRes;
  const MeqResult& ampl  = itsAmpl.getResultSynced (request, amplRes);
  const MeqResult& phase = itsPhase.getResultSynced (request, phaseRes);
  MeqResult result(request.nspid());
  MeqMatrixTmp matt (tocomplex(cos(phase.getValue()), sin(phase.getValue())));
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    if (phase.isDefined(spinx)) {
      const MeqMatrix& ph = phase.getPerturbedValue(spinx);
      result.setPerturbedValue (spinx,
				ampl.getPerturbedValue(spinx) *
				tocomplex(cos(ph), sin(ph)));
      result.setPerturbation (spinx, phase.getPerturbation(spinx));
    } else if (ampl.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
				ampl.getPerturbedValue(spinx) *
				matt.clone());
      result.setPerturbation (spinx, ampl.getPerturbation(spinx));
    }
  }
  result.setValue (matt * ampl.getValue());
  return result;
}

}
