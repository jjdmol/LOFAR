//# MeqJonesNode.cc: A node in a Jones matrix expression.
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

#include <Common/Profiling/PerfProfile.h>

#include <PSS3/MNS/MeqJonesNode.h>
#include <PSS3/MNS/MeqExpr.h>

namespace LOFAR {

MeqJonesNode::MeqJonesNode (MeqExpr* elem11, MeqExpr* elem12,
			    MeqExpr* elem21, MeqExpr* elem22)
: itsExpr11 (elem11),
  itsExpr12 (elem12),
  itsExpr21 (elem21),
  itsExpr22 (elem22)
{}

MeqJonesNode::~MeqJonesNode()
{}

void MeqJonesNode::calcResult (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  setResult11 (itsExpr11->getResult (request));
  setResult12 (itsExpr12->getResult (request));
  setResult21 (itsExpr21->getResult (request));
  setResult22 (itsExpr22->getResult (request));
}

}
