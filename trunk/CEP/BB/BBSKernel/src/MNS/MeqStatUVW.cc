//# MeqStatUVW.cc: The station UVW coordinates for a time domain
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

#include <lofar_config.h>

#include <BBS/MNS/MeqStatUVW.h>
#include <BBS/MNS/MeqStation.h>
#include <BBS/MNS/MeqPhaseRef.h>
#include <BBS/MNS/MeqRequest.h>
#include <BBS/MNS/MeqExpr.h>
#include <BBS/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <casa/Quanta/MVuvw.h>
#include <iomanip>

using namespace casa;

namespace LOFAR {

MeqStatUVW::MeqStatUVW (MeqStation* station,
			const MeqPhaseRef* phaseRef)
: itsStation   (station),
  itsPhaseRef  (phaseRef),
  itsU         (0),
  itsV         (0),
  itsW         (0),
  itsLastReqId (InitMeqRequestId)
{
  itsFrame.set (itsPhaseRef->direction());
  itsFrame.set (itsPhaseRef->earthPosition());
}

void MeqStatUVW::clear()
{
  itsLastReqId = InitMeqRequestId;
  itsUVW.clear();
  itsU.clear();
  itsV.clear();
  itsW.clear();
}

void MeqStatUVW::calculate (const MeqRequest& request)
{
  // Make sure the MeqResult/Matrix objects have the correct size.
  int ntime = request.ny();
  double* uptr = itsU.setDoubleFormat (1, ntime);
  double* vptr = itsV.setDoubleFormat (1, ntime);
  double* wptr = itsW.setDoubleFormat (1, ntime);
  // Use the UVW coordinates if already calculated for a time.
  int ndone = 0;
  for (int i=0; i<ntime; ++i) {
    MeqTime meqtime(request.y(i));
    map<MeqTime,MeqUVW>::iterator pos = itsUVW.find (meqtime);
    if (pos != itsUVW.end()) {
      uptr[i] = pos->second.itsU;
      vptr[i] = pos->second.itsV;
      wptr[i] = pos->second.itsW;
      ndone++;
    }
  }
  // If all found we can exit.
  if (ndone == ntime) {
    itsLastReqId = request.getId();
    return;
  }
  // Calculate the other UVW coordinates using the AIPS++ code.
  // First form the time-independent stuff.
  ASSERTSTR (itsStation, "UVW coordinates cannot be calculated");
  MeqResult posx = itsStation->getPosX().getResult (request);
  MeqResult posy = itsStation->getPosY().getResult (request);
  MeqResult posz = itsStation->getPosZ().getResult (request);
  LOG_TRACE_FLOW_STR ("posx" << posx.getValue());
  LOG_TRACE_FLOW_STR ("posy" << posy.getValue());
  LOG_TRACE_FLOW_STR ("posz" << posz.getValue());
  // Get position relative to center to keep values small.
  const MVPosition& mvcpos = itsPhaseRef->earthPosition().getValue();
  MVPosition mvpos(posx.getValue().getDouble() - mvcpos(0),
		   posy.getValue().getDouble() - mvcpos(1),
		   posz.getValue().getDouble() - mvcpos(2));
  MVBaseline mvbl(mvpos);
  MBaseline mbl(mvbl, MBaseline::ITRF);
  LOG_TRACE_FLOW_STR ("mbl " << mbl);
  Quantum<double> qepoch(0, "s");
  MEpoch mepoch(qepoch, MEpoch::UTC);
  itsFrame.set (mepoch);
  mbl.getRefPtr()->set(itsFrame);      // attach frame
  MBaseline::Convert mcvt(mbl, MBaseline::J2000);
  for (int i=0; i<ntime; ++i) {
    double time = request.y(i);
    map<MeqTime,MeqUVW>::iterator pos = itsUVW.find (MeqTime(time));
    if (pos == itsUVW.end()) {
      qepoch.setValue (time);
      LOG_TRACE_FLOW_STR ("frame " << mbl.getRefPtr()->getFrame());
      const MVBaseline& bas2000 = mcvt().getValue();
      LOG_TRACE_FLOW_STR (bas2000);
      LOG_TRACE_FLOW_STR (itsPhaseRef->direction().getValue());
      MVuvw uvw2000 (bas2000, itsPhaseRef->direction().getValue());
      const Vector<double>& xyz = uvw2000.getValue();
      LOG_TRACE_FLOW_STR (xyz(0) << ' ' << xyz(1) << ' ' << xyz(2));
      *uptr++ = xyz(0);
      *vptr++ = xyz(1);
      *wptr++ = xyz(2);
      // Save the UVW coordinates in the map.
      itsUVW[MeqTime(time)] = MeqUVW(xyz(0), xyz(1), xyz(2));
///    if (itsStation->getName() == "SR9" || itsStation->getName() == "SR12") {
///  cout << "UVW="<<itsStation->getName()<<' ' << std::setprecision(12)<<xyz(0) << ' '<< std::setprecision(12)<<xyz(1)<<' '<< std::setprecision(12)<<xyz(2) << ' '<<std::setprecision(12)<<time<<' '<<mvcpos(0)<<' '<<mvcpos(1)<<' '<<mvcpos(2)<<' ' <<posz.getValue()<<' '<<posy.getValue()<<' '<<posz.getValue()<<endl;
///    }
     }
  }
  LOG_TRACE_FLOW_STR ('U' << itsU.getValue());
  LOG_TRACE_FLOW_STR ('V' << itsV.getValue());
  LOG_TRACE_FLOW_STR ('W' << itsW.getValue());
  itsLastReqId = request.getId();
}

void MeqStatUVW::set (double time, double u, double v, double w)
{
  itsUVW[MeqTime(time)] = MeqUVW(u,v,w);
}

}
