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

//# Includes
#include <PSS3/MNS/MeqJonesExpr.h>

//# Forward Declarations
class MeqExpr;


// This class is a node in a Jones matrix expression.

class MeqStatExpr: public MeqJonesExpr
{
public:
  // Construct from the various subexpressions.
  MeqStatExpr (MeqExpr* faradayRotation,
	       MeqExpr* dipoleRotation,
	       MeqExpr* dipoleEllipticity,
	       MeqExpr* gain1,
	       MeqExpr* gain2);

  virtual ~MeqStatExpr();

  // Calculate the result of its members.
  virtual void calcResult (const MeqRequest&);

private:
  MeqExpr* itsFarRot;
  MeqExpr* itsDipRot;
  MeqExpr* itsDipEll;
  MeqExpr* itsGain1;
  MeqExpr* itsGain2;
};


#endif
