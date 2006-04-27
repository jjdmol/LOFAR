//# MeqJonesDiag.cc: A diagonal node in a Jones matrix expression.
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
#include <Common/Profiling/PerfProfile.h>

#include <BBS/MNS/MeqJonesDiag.h>
#include <BBS/MNS/MeqExpr.h>

namespace LOFAR {

MeqJonesDiag::MeqJonesDiag (const MeqExpr& elem11, const MeqExpr& elem22)
: itsExpr11 (elem11),
  itsExpr22 (elem22)
{
  addChild (itsExpr11);
  addChild (itsExpr22);
}

MeqJonesDiag::~MeqJonesDiag()
{}

MeqJonesResult MeqJonesDiag::getJResult (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  MeqJonesResult res(0);
  {
    itsExpr11.getResultSynced (request, res.result11());
    itsExpr22.getResultSynced (request, res.result22());
  }
  return res;
}

}
