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

//# Includes
#include <MNS/MeqExpr.h>
#include <MNS/MeqResult.h>
#include <MNS/MeqRequest.h>

//# Forward Declarations
class MeqPhaseRef;


class MeqPointSource
{
public:
  // The default constructor.
  MeqPointSource();

  MeqPointSource (MeqExpr* fluxI, MeqExpr* fluxQ,
		  MeqExpr* fluxU, MeqExpr* fluxV,
		  MeqExpr* ra, MeqExpr* dec);

  MeqExpr* getI()
    { return itsI; }
  MeqExpr* getQ()
    { return itsQ; }
  MeqExpr* getU()
    { return itsU; }
  MeqExpr* getV()
    { return itsV; }

  // Set the source nr.
  void setSourceNr (int sourceNr)
    { itsSourceNr = sourceNr; }

  // Set the phase reference position.
  void setPhaseRef (const MeqPhaseRef* phaseRef)
    { itsPhaseRef = phaseRef; }

  // Get the precalculated result of l, m, or n.
  const MeqResult& getL (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsL; }
  const MeqResult& getM (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsM; }
  const MeqResult& getN (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsN; }
  
  void calculate (const MeqRequest&);

private:
  int       itsSourceNr;
  MeqExpr*  itsI;
  MeqExpr*  itsQ;
  MeqExpr*  itsU;
  MeqExpr*  itsV;
  MeqExpr*  itsRa;
  MeqExpr*  itsDec;
  const MeqPhaseRef* itsPhaseRef;
  MeqResult itsL;
  MeqResult itsM;
  MeqResult itsN;
  MeqRequestId itsLastReqId;
};


#endif
