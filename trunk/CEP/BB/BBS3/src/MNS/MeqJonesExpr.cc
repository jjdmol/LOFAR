//# MeqJonesExpr.cc: The base class of a Jones matrix expression.
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
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqJonesExpr.h>
#include <Common/LofarLogger.h>
//#include <Common/Timer.h>

namespace LOFAR {

MeqJonesExprRep::~MeqJonesExprRep()
{
  delete itsResult;
}

MeqJonesExpr::MeqJonesExpr (const MeqJonesExpr& that)
: itsRep (that.itsRep)
{
  if (itsRep != 0) {
    itsRep->link();
  }
}
MeqJonesExpr& MeqJonesExpr::operator= (const MeqJonesExpr& that)
{
  if (this != &that) {
    MeqJonesExprRep::unlink (itsRep);
    itsRep = that.itsRep;
    if (itsRep != 0) {
      itsRep->link();
    }
  }
  return *this;
}

const MeqJonesResult& MeqJonesExprRep::calcResult (const MeqRequest& request,
						   MeqJonesResult& result)
{
  // The value has to be calculated.
  // Do not cache if no multiple parents.
  if (itsNParents <= 1) {
    result = getResult (request);
    return result;
  }

  // Use a cache.
  // Synchronize the calculations.
  // Only calculate if not already calculated in another thread.
  //static NSTimer timer("MeqJonesExprRep::calcResult", true);
  //timer.start();
#if defined _OPENMP
#pragma omp critical(calcResult)
  if (itsReqId != request.getId())	// retry test in critical section
#endif
  {
    if (!itsResult) itsResult = new MeqJonesResult;
    *itsResult = getResult (request);
    itsReqId = request.getId();
  }
  //timer.stop();

  return *itsResult;
}

}
