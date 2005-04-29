//# MeqPointSource.h: Class holding a point source
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

#if !defined(MNS_MEQPOINTSOURCE_H)
#define MNS_MEQPOINTSOURCE_H

// \file MNS/MeqPointSource.h
// Class holding a point source

//# Includes
#include <BBS3/MNS/MeqExpr.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqRequest.h>
#include <Common/lofar_string.h>

namespace LOFAR {

// \ingroup BBS3
// \addtogroup MNS
// @{

//# Forward Declarations
class MeqPhaseRef;


class MeqPointSource
{
public:
  // The default constructor.
  MeqPointSource();

  MeqPointSource (const string& name,
		  const MeqExpr& fluxI, const MeqExpr& fluxQ,
		  const MeqExpr& fluxU, const MeqExpr& fluxV,
		  const MeqExpr& ra, const MeqExpr& dec);

  const string& getName() const
    { return itsName; }

  MeqExpr& getI()
    { return itsI; }
  MeqExpr& getQ()
    { return itsQ; }
  MeqExpr& getU()
    { return itsU; }
  MeqExpr& getV()
    { return itsV; }
  MeqExpr& getRa()
    { return itsRa; }
  MeqExpr& getDec()
    { return itsDec; }

  // Get the source nr.
  int getSourceNr() const
    { return itsSourceNr; }

  // Get the group nr.
  int getGroupNr() const
    { return itsGroupNr; }

  // Set the source nr.
  void setSourceNr (int sourceNr)
    { itsSourceNr = sourceNr; }

  // Set the group nr.
  void setGroupNr (int groupNr)
    { itsGroupNr = groupNr; }

private:
  int       itsSourceNr;
  int       itsGroupNr;
  string    itsName;
  MeqExpr   itsI;
  MeqExpr   itsQ;
  MeqExpr   itsU;
  MeqExpr   itsV;
  MeqExpr   itsRa;
  MeqExpr   itsDec;
};

// @}

}

#endif
