//# MeqStatExpr.h: The Jones expression for a station
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

#if !defined(MNS_MEQSTATEXPR_H)
#define MNS_MEQSTATEXPR_H

// \file MNS/MeqStatExpr.h
// The Jones expression for a station

//# Includes
#include <BBS3/MNS/MeqJonesExpr.h>
#include <BBS3/MNS/MeqExpr.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward Declarations
class MeqExpr;

// This class is a node in a Jones matrix expression.

class MeqStatExpr: public MeqJonesExprRep
{
public:
  // Construct from the various subexpressions.
  MeqStatExpr (const MeqExpr& faradayRotation,
	       const MeqExpr& dipoleRotation,
	       const MeqExpr& dipoleEllipticity,
	       const MeqExpr& gain1,
	       const MeqExpr& gain2);

  virtual ~MeqStatExpr();

  // Calculate the result of its members.
  virtual MeqJonesResult getResult (const MeqRequest&);

private:
  MeqExpr itsFarRot;
  MeqExpr itsDipRot;
  MeqExpr itsDipEll;
  MeqExpr itsGain1;
  MeqExpr itsGain2;
};

// @}

}

#endif
