//# MeqDiag.cc: A diagonal node in a Jones matrix expression.
//#
//# Copyright (C) 2005
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
//#include <Common/Profiling/PerfProfile.h>

#include <BBSKernel/MNS/MeqDiag.h>
#include <BBSKernel/MNS/MeqExpr.h>

namespace LOFAR
{
namespace BBS
{

MeqDiag::MeqDiag(const MeqExpr& xx, const MeqExpr& yy)
: itsXX(xx),
  itsYY(yy)
{
  addChild(itsXX);
  addChild(itsYY);
}

MeqDiag::~MeqDiag()
{}

MeqJonesResult MeqDiag::getJResult(const MeqRequest& request)
{
//  PERFPROFILE(__PRETTY_FUNCTION__);

  MeqJonesResult res(0);
  {
    itsXX.getResultSynced(request, res.result11());
    itsYY.getResultSynced(request, res.result22());
    res.result12().setValue (MeqMatrix(0.));
    res.result21().setValue (MeqMatrix(0.));
  }
  return res;
}

} // namespace BBS
} // namespace LOFAR
