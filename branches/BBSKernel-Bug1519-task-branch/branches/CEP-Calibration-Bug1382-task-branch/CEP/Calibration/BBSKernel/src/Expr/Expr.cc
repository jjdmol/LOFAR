//# Expr.cc: The base class of an expression.
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
#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/Result.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/PValueIterator.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
//#include <Common/Timer.h>
#include <iomanip>
#include <algorithm>

namespace LOFAR
{
namespace BBS
{

ExprRep::ExprRep()
  : itsCount    (0),
    itsMinLevel (100000000),     // very high number
    itsMaxLevel (-1),
    itsLevelDone(-1),
    itsResult   (0),
    itsResVec   (0),
    itsNParents (0),
    itsReqId    (InitRequestId)
{}

ExprRep::~ExprRep()
{
  for(std::vector<Expr>::iterator it = itsChildren.begin(); it != itsChildren.end(); ++it)
  {
      ExprRep *childRep = (*it).itsRep;
      ASSERT(childRep);
      childRep->decrNParents();
  }

  delete itsResult;
  delete itsResVec;
}

void ExprRep::addChild (Expr child)
{
  ExprRep *childRep = child.itsRep;
  ASSERT(childRep);
  childRep->incrNParents();
  itsChildren.push_back(child);
}

void ExprRep::removeChild(Expr child)
{
    std::vector<Expr>::iterator it = std::find(itsChildren.begin(), itsChildren.end(), child);
    ASSERT(it != itsChildren.end());

    ExprRep *childRep = child.itsRep;
    ASSERT(childRep);

    childRep->decrNParents();
    itsChildren.erase(it);
}

Expr ExprRep::getChild(size_t index)
{
    ASSERT(index < itsChildren.size());
    return itsChildren[index];
}

int ExprRep::setLevel (int level)
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

void ExprRep::clearDone()
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

void ExprRep::getCachingNodes (std::vector<ExprRep*>& nodes,
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

void ExprRep::precalculate (const Request& request)
{
  ResultVec result;
  calcResultVec (request, result, true);
  DBGASSERT (itsResVec);
  if (itsResVec->size() == 1) {
    if (!itsResult) itsResult = new Result;
    *itsResult = (*itsResVec)[0];
  }
}

const Result& ExprRep::calcResult (const Request& request,
                     Result& result)
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
  //static NSTimer timer("ExprRep::calcResult", true);
  //timer.start();
#if defined _OPENMP
#pragma omp critical(calcResult)
  if (itsReqId != request.getId())  // retry test in critical section
#endif
  {
    if (!itsResult) itsResult = new Result;
    *itsResult = getResult (request);
    itsReqId = request.getId();
  }
  //timer.stop();

  return *itsResult;
}

Matrix ExprRep::getResultValue (const Request &, const vector<const Matrix*>&)
{
  THROW (LOFAR::Exception,
     "Expr::getResult(Value) not implemented in derived class");
}

Result ExprRep::getResult (const Request& request)
{
  // Calculate the result in a generic way.
  Result result;
  result.init();

  // First evaluate all children and keep their results.
  // Also keep the main value.
  size_t nrchild = itsChildren.size();
  vector<Result> buf(nrchild);
  vector<const Result*> res(nrchild);
  vector<const Matrix*> mat(nrchild);
  for (size_t i=0; i<nrchild; ++i) {
    res[i] = &itsChildren[i].getResultSynced (request, buf[i]);
    mat[i] = &res[i]->getValue();
  }

  // Calculate the resulting main value.
  result.setValue (getResultValue(request, mat));

  // Calculate the perturbed values.
  PValueSetIteratorDynamic pvIter(res);

  while(!pvIter.atEnd())
  {
    for (size_t i=0; i<nrchild; ++i) {
      mat[i] = &(pvIter.value(i));
    }

    result.setPerturbedValue(pvIter.key(), getResultValue(request, mat));
    pvIter.next();
  }

  return result;
}

const ResultVec& ExprRep::calcResultVec (const Request& request,
                           ResultVec& result,
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
  //static NSTimer timer("ExprRep::calcResultVec", true);
  //timer.start();
#if defined _OPENMP
#pragma omp critical(calcResult)
  if (itsReqId != request.getId())  // retry test in critical section
#endif
  {
    if (!itsResVec) itsResVec = new ResultVec;
    *itsResVec = getResultVec (request);
    itsReqId = request.getId();
  }
  //timer.stop();

  return *itsResVec;
}

ResultVec ExprRep::getResultVec (const Request& request)
{
  ResultVec res(1);
  res[0] = getResult (request);
  return res;
}



Expr::Expr (const Expr& that)
: itsRep (that.itsRep)
{
  if (itsRep != 0) {
    itsRep->link();
  }
}
Expr& Expr::operator= (const Expr& that)
{
  if (this != &that) {
    ExprRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

ExprToComplex::ExprToComplex (const Expr& real,
                    const Expr& imag)
  : itsReal(real),
    itsImag(imag)
{
  addChild (itsReal);
  addChild (itsImag);
}

ExprToComplex::~ExprToComplex()
{}

Result ExprToComplex::getResult (const Request& request)
{
  Result realRes;
  Result imagRes;
  const Result& real = itsReal.getResultSynced (request, realRes);
  const Result& imag = itsImag.getResultSynced (request, imagRes);

  // Compute main value.
  Result result;
  result.init();
  result.setValue(tocomplex(real.getValue(), imag.getValue()));

  // Compute perturbed values.
  const Result *pvSet[2] = {&real, &imag};
  PValueSetIterator<2> pvIter(pvSet);

  while(!pvIter.atEnd())
  {
    result.setPerturbedValue(pvIter.key(), tocomplex(pvIter.value(0),
        pvIter.value(1)));
    pvIter.next();
  }

  return result;
}


ExprAPToComplex::ExprAPToComplex (const Expr& ampl,
                    const Expr& phase)
  : itsAmpl (ampl),
    itsPhase(phase)
{
  addChild (itsAmpl);
  addChild (itsPhase);
}

ExprAPToComplex::~ExprAPToComplex()
{}

Result ExprAPToComplex::getResult (const Request& request)
{
  Result amplRes;
  Result phaseRes;
  const Result& ampl  = itsAmpl.getResultSynced (request, amplRes);
  const Result& phase = itsPhase.getResultSynced (request, phaseRes);

  // Allocate result.
  Result result;
  result.init();

  MatrixTmp matt(tocomplex(cos(phase.getValue()), sin(phase.getValue())));

  // Compute perturbed values.
  const Result *pvSet[2] = {&ampl, &phase};
  PValueSetIterator<2> pvIter(pvSet);

  while(!pvIter.atEnd())
  {
    if(pvIter.hasPValue(1)) {
      const Matrix &pvPhase = pvIter.value(1);
      result.setPerturbedValue(pvIter.key(), pvIter.value(0)
        * tocomplex(cos(pvPhase), sin(pvPhase)));
    }
    else {
      result.setPerturbedValue(pvIter.key(), pvIter.value(0) * matt.clone());
    }

    pvIter.next();
  }

  // Compute main value.
  // NB. Order is important because matt is of type MatrixTmp which is modified
  // in place.
  result.setValue (matt * ampl.getValue());

  return result;
}


ExprPhaseToComplex::ExprPhaseToComplex(const Expr &phase)
    : itsPhase(phase)
{
    addChild(itsPhase);
}

ExprPhaseToComplex::~ExprPhaseToComplex()
{
}

Result ExprPhaseToComplex::getResult(const Request &request)
{
    Result tmpPhase;
    const Result &phase = itsPhase.getResultSynced(request, tmpPhase);

    // Allocate result.
    Result result;
    result.init();

    // Compute main value.
    result.setValue(tocomplex(cos(phase.getValue()), sin(phase.getValue())));

    // Compute perturbed values.
    const Result *pvSet[1] = {&phase};
    PValueSetIterator<1> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        const Matrix &pvPhase = pvIter.value(0);
        result.setPerturbedValue(pvIter.key(), tocomplex(cos(pvPhase),
            sin(pvPhase)));
        pvIter.next();
    }

    return result;
}


} // namespace BBS
} // namespace LOFAR
