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
#include <BBSKernel/MNS/MeqExpr.h>
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
//#include <Common/Timer.h>
#include <iomanip>
#include <algorithm>

namespace LOFAR
{
namespace BBS
{

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
  for(std::vector<MeqExpr>::iterator it = itsChildren.begin(); it != itsChildren.end(); ++it)
  {
      MeqExprRep *childRep = (*it).itsRep;
      ASSERT(childRep);
      childRep->decrNParents();
  }
  
  delete itsResult;
  delete itsResVec;
}

void MeqExprRep::addChild (MeqExpr child)
{
  MeqExprRep *childRep = child.itsRep;
  ASSERT(childRep);
  childRep->incrNParents();
  itsChildren.push_back(child);
}

void MeqExprRep::removeChild(MeqExpr child)
{
    std::vector<MeqExpr>::iterator it = std::find(itsChildren.begin(), itsChildren.end(), child);
    ASSERT(it != itsChildren.end());
    
    MeqExprRep *childRep = child.itsRep;
    ASSERT(childRep);
    
    childRep->decrNParents();
    itsChildren.erase(it);
}

MeqExpr MeqExprRep::getChild(size_t index)
{
    ASSERT(index < itsChildren.size());
    return itsChildren[index];
}

int MeqExprRep::setLevel (int level)
{
  if (level < itsMinLevel) itsMinLevel = level;
  int nrLev = itsMaxLevel;
  if (level > itsMaxLevel) {
    nrLev = level;
    itsMaxLevel = level;
    for (uint i=0; i<itsChildren.size(); ++i) {
      nrLev = std::max(nrLev, itsChildren[i].setLevel (level+1));
    }
  }
  return nrLev;
}

void MeqExprRep::clearDone()
{
  itsLevelDone = -1;
  itsMaxLevel  = -1;
  itsMinLevel  = 100000000;
  for (uint i=0; i<itsChildren.size(); ++i) {
    // Avoid that a child is cleared multiple times.
    if (itsChildren[i].levelDone() >= 0) {
      itsChildren[i].clearDone();
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
    if (itsChildren[i].levelDone() != level) {
      itsChildren[i].getCachingNodes (nodes, level, all);
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
  if (itsReqId != request.getId())  // retry test in critical section
#endif
  {
    if (!itsResult) itsResult = new MeqResult;
    *itsResult = getResult (request);
    itsReqId = request.getId();
  }
  //timer.stop();

  return *itsResult;
}

MeqMatrix MeqExprRep::getResultValue (const vector<const MeqMatrix*>&)
{
  THROW (LOFAR::Exception,
     "MeqExpr::getResult(Value) not implemented in derived class");
}

MeqResult MeqExprRep::getResult (const MeqRequest& request)
{
  // Calculate the result in a generic way.
  MeqResult result(request.nspid());
  // First evaluate all children and keep their results.
  // Also keep the main value.
  int nrchild = itsChildren.size();
  vector<MeqResult> res(nrchild);
  vector<const MeqMatrix*> mat(nrchild);
  for (int i=0; i<nrchild; ++i) {
    res[i] = itsChildren[i].getResultSynced (request, res[i]);
    mat[i] = &res[i].getValue();
  }
  // Calculate the resulting main value.
  result.setValue (getResultValue(mat));
  // Calculate the perturbed values for which any child is perturbed.
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    int eval = -1;
    for (int i=0; i<nrchild; ++i) {
      if (res[i].isDefined(spinx)) {
    // Perturbed, so set value to the perturbed one.
    mat[i] = &res[i].getPerturbedValue(spinx);
    eval = i;
      } else {
    mat[i] = &res[i].getValue();
      }
    }
    if (eval >= 0) {
      // Evaluate the perturbed value and reset the value vector.
      result.setPerturbedParm (spinx, res[eval].getPerturbedParm(spinx));
      result.setPerturbedValue (spinx, getResultValue(mat));
    }
  }
  return result;
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
  if (itsReqId != request.getId())  // retry test in critical section
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

#ifdef EXPR_GRAPH
std::string MeqExprRep::getLabel()
{
    return std::string("MeqExprRep base class");
}

void MeqExprRep::writeExpressionGraph(std::ostream &os)
{
    os << "id" << std::hex << this << " [label=\"" << getLabel() << "\"];" << std::endl;

    std::vector<MeqExpr>::iterator it;
    for(it = itsChildren.begin(); it != itsChildren.end(); ++it)
    {
          os << "id" << std::hex << it->rep() << " -> " << "id" << std::hex
            << this << ";" << std::endl;
          it->writeExpressionGraph(os);
    }
}
#endif


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
      result.setPerturbedParm (spinx, real.getPerturbedParm(spinx));
    } else if (imag.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
                tocomplex(real.getPerturbedValue(spinx),
                      imag.getPerturbedValue(spinx)));
      result.setPerturbedParm (spinx, imag.getPerturbedParm(spinx));
    }
  }
  result.setValue (tocomplex(real.getValue(), imag.getValue()));
  return result;
}

#ifdef EXPR_GRAPH
std::string MeqExprToComplex::getLabel()
{
    return std::string("MeqExprToComplex\\nreal / imaginary");
}
#endif

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
      result.setPerturbedParm (spinx, phase.getPerturbedParm(spinx));
    } else if (ampl.isDefined(spinx)) {
      result.setPerturbedValue (spinx,
                ampl.getPerturbedValue(spinx) *
                matt.clone());
      result.setPerturbedParm (spinx, ampl.getPerturbedParm(spinx));
    }
  }
  result.setValue (matt * ampl.getValue());
  return result;
}

#ifdef EXPR_GRAPH
std::string MeqExprAPToComplex::getLabel()
{
    return std::string("MeqExprToComplex\\namplitude / phase");
}
#endif

} // namespace BBS
} // namespace LOFAR
