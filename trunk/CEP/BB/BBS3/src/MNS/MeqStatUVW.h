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
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqRequest.h>
#include <Common/lofar_map.h>
#include <measures/Measures/MeasFrame.h>

namespace LOFAR {

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

  // Clear the map of UVW coordinates and reset the last request id.
  void clear();

  // Set the UVW coordinate for the given time.
  void set (double time, double u, double v, double w);

private:
  struct MeqTime
  {
    MeqTime (double time) :itsTime(time) {}
    bool operator< (const MeqTime& other) const
      { return itsTime < other.itsTime-0.000001; }
    //operator== (const MeqTime& other)
    //    { return itsTime > other.itsTime-0.001 && itsTime < other.itsTime+0.001; }
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

  MeqStation*  itsStation;
  const MeqPhaseRef* itsPhaseRef;
  MeqResult    itsU;
  MeqResult    itsV;
  MeqResult    itsW;
  MeqRequestId itsLastReqId;
  MeasFrame    itsFrame;
  map<MeqTime,MeqUVW> itsUVW;    // association of time and UVW coordinates
};

}

#endif
