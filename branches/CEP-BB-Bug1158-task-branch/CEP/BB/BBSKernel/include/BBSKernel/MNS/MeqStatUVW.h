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

#ifndef MNS_MEQSTATUVW_H
#define MNS_MEQSTATUVW_H

// \file
// The station UVW coordinates for a time domain

//# Includes
#include <BBSKernel/MNS/MeqResult.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqStation.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <utility>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>


namespace LOFAR
{
namespace BBS
{

// \ingroup BBSKernel
// \addtogroup MNS
// @{

//# Forward declarations
class MeqStation;

class MeqStatUVW
{
public:
  // The default constructor.
  MeqStatUVW() {};

//#   MeqStatUVW (MeqStation*, const MeqPhaseRef*);
  MeqStatUVW(MeqStation *station, const casa::MDirection &phaseCenter,
    const casa::MPosition &arrayPosition);

  void calculate (const MeqRequest&);

  // Get the U, V or W value.
  const MeqResult& getU (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsU; }
  const MeqResult& getV (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsV; }
  const MeqResult& getW (const MeqRequest& request)
    { if (request.getId() != itsLastReqId) calculate(request); return itsW; }

  // Set the UVW coordinate for the given time.
  void set (double time, double u, double v, double w);

#ifdef EXPR_GRAPH
  const MeqStation *getStation() const
  {
      return itsStation;
  }
#endif
  
private:
  struct MeqTime
  {
    MeqTime (double time) :itsTime(time) {}
    bool operator< (const MeqTime& other) const
        { return itsTime < other.itsTime-0.000001; }
    //# operator== (const MeqTime& other)
    //#    { return itsTime > other.itsTime-0.001 && itsTime < other.itsTime+0.001; }
    double itsTime;
  };

  struct MeqUVW
  {
    MeqUVW() {}
    MeqUVW (double u, double v, double w) : itsU(u), itsV(v), itsW(w) {}
    double itsU;
    double itsV;
    double itsW;
  };

  MeqStation            *itsStation;
  casa::MDirection      itsPhaseCenter;
  casa::MPosition       itsArrayPosition;
  MeqResult             itsU;
  MeqResult             itsV;
  MeqResult             itsW;
  MeqRequestId          itsLastReqId;
  map<MeqTime,MeqUVW>   itsUVW;    // association of time and UVW coordinates

//#  const MeqPhaseRef* itsPhaseRef;
//#  casa::MeasFrame    itsFrame;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
