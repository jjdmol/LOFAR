//# MeqPhaseRef.h: Phase reference position and derived values
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

#if !defined(MNS_MEQPHASEREF_H)
#define MNS_MEQPHASEREF_H

//# Includes
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

//# Forward Declarations
class MDirection;


class MeqPhaseRef
{
public:
  // The default constructor.
  MeqPhaseRef() {};

  MeqPhaseRef (const MDirection& phaseRef, double startTime);

  double getRa() const
    { return itsRa; }
  double getDec() const
    { return itsDec; }
  double getSinDec() const
    { return itsSinDec; }
  double getCosDec() const
    { return itsCosDec; }
  double getStartTime() const
    { return itsStartTime; }
  double getStartHA() const
    { return itsStartHA; }
  double getScaleHA() const
    { return itsScaleHA; }

  const MDirection& direction() const
    { return itsDir; }
  const MPosition& earthPosition() const
    { return itsEarthPos; }

private:
  double itsRa;
  double itsDec;
  double itsSinDec;
  double itsCosDec;
  double itsStartTime;
  double itsStartHA;
  double itsScaleHA;
  MDirection itsDir;
  MPosition  itsEarthPos;
};


#endif
