//# MeqStatUVW.h: The station UVW coordinates for a time domain
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

#if !defined(MNS_MEQSTATUVW_H)
#define MNS_MEQSTATUVW_H

//# Includes
#include <MNS/MeqResult.h>
#include <MNS/MeqRequest.h>
#include <aips/Measures/MeasFrame.h>

//# Forward declarations
class MeqStation;
class MeqPhaseRef;


class MeqStatUVW
{
public:
  // The default constructor.
  MeqStatUVW() {};

  MeqStatUVW (MeqStation*, const MeqPhaseRef*);

  void calculate (const MeqRequest&);

  // Get the U, V or W value.
  const MeqResult& getU (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsU; }
  const MeqResult& getV (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsV; }
  const MeqResult& getW (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsW; }

private:
  MeqStation*  itsStation;
  const MeqPhaseRef* itsPhaseRef;
  MeqResult    itsU;
  MeqResult    itsV;
  MeqResult    itsW;
  MeqRequestId itsLastReqId;
  MeasFrame    itsFrame;
};


#endif
