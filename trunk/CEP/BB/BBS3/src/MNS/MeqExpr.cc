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
//#include <Common/Timer.h>

namespace LOFAR {

MeqExprRep::MeqExprRep()
  : itsCount    (0),
    itsMinLevel (100000000),     // very high number
    itsMaxLevel (-1),
    itsLevelDone(-1),
    itsResult   (0),
    itsResVec   (0),
    itsNParents (0),
    itsReqId    (InitMeqRequestId)
{}

MeqExprRep::~MeqExprRep()
{
  delete itsResult;
  delete itsResVec;
}

void MeqExprRep::addChild (MeqExpr& child)
{
  MeqExprRep* childRep = child.itsRep;
  ASSERT (childRep != 0);
  itsChildren.push_back (childRep);
  childRep->incrNParents();
}

int MeqExprRep::setLevel (int level)
{
  if (level < itsMinLevel) itsMinLevel = level;
  int nrLev = itsMaxLevel;
  if (level > itsMaxLevel) {
    nrLev = level;
    itsMaxLevel = level;
    for (uint i=0; i<itsChildren.size(); ++i) {
      nrLev = std::max(nrLev, itsChildren[i]->setLevel (level+1));
    }
  }
  return nrLev;
}

void MeqExprRep::clearDone()
{
  itsLevelDone = -1;
  for (uint i=0; i<itsChildren.size(); ++i) {
    // Avoid that a child is cleared multiple times.
    if (itsChildren[i]->levelDone() >= 0) {
      itsChildren[i]->clearDone();
    }
  }
}

void MeqExprRep::getCachingNodes (std::vector<MeqExprRep*>& nodes,
				  int level, bool all)
{
  if (itsLevelDone != level) {
    if (itsMaxLevel == level) {
      // Possibly add this node.
      if (all  ||  itsNParents > 1) {
	nodes.push_back (this);
      }
    } else if (itsMaxLevel < level) {
      // Handling the children is needed.
      for (uint i=0; i<itsChildren.size(); ++i) {
	if (itsChildren[i]->levelDone() != level) {
	  itsChildren[i]->getCachingNodes (nodes, level, all);
	}
      }
    }
    itsLevelDone = level;
  }
}

void MeqExprRep::precalculate (const MeqRequest& request)
{
  MeqResultVec result;
  calcResultVec (request, result, true);
  DBGASSERT (itsResVec);
  if (itsResVec->nresult() == 1) {
    if (!itsResult) itsResult = new MeqResult;
    *itsResult = (*itsResVec)[0];
  }
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
  // It should never come past this.
  ASSERT(false);

  // Use a cache.
  // Synchronize the calculations.
  // Only calculate if not already calculated in another thread.
  //static NSTimer timer("MeqExprRep::calcResult", true);
  //timer.start();
#if defined _OPENMP
#pragma omp critical(calcResult)
  if (itsReqId != request.getId())	// retry test in critical section
#endif
  {
    if (!itsResult) itsResult = new MeqResult;
    *itsResult = getResult (request);
    itsReqId = request.getId();
  }
  //timer.stop();

  return *itsResult;
}

MeqResult MeqExprRep::getResult (const MeqRequest&)
{
  THROW (LOFAR::Exception,
	 "MeqExpr::getResult not implemented in derived class");
}

const MeqResultVec& MeqExprRep::calcResultVec (const MeqRequest& request,
					       MeqResultVec& result,
					       bool useCache)
{
  // The value has to be calculated.
  // Do not cache if no multiple parents.
  if (itsNParents <= 1  &&  !useCache) {
    result = getResultVec (request);
    return result;
  }
  // It should never come past this (unless called from precalculate).
  ASSERT(useCache);

  // Use a cache.
  // Synchronize the calculations.
  // Only calculate if not already calculated in another thread.
  //static NSTimer timer("MeqExprRep::calcResultVec", true);
  //timer.start();
#if defined _OPENMP
#pragma omp critical(calcResult)
  if (itsReqId != request.getId())	// retry test in critical section
#endif
  {
    if (!itsResVec) itsResVec = new MeqResultVec;
    *itsResVec = getResultVec (request);
    itsReqId = request.getId();
  }
  //timer.stop();

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



MeqExprToComplex::MeqExprToComplex (const MeqExpr& real,
				    const MeqExpr& imag)
  : itsReal(real),
    itsImag(imag)
{
  addChild (itsReal);
  addChild (itsImag);
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




MeqExprAPToComplex::MeqExprAPToComplex (const MeqExpr& ampl,
					const MeqExpr& phase)
  : itsAmpl (ampl),
    itsPhase(phase)
{
  addChild (itsAmpl);
  addChild (itsPhase);
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
