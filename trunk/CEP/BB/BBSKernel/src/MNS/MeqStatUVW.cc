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

#include <BBSKernel/MNS/MeqStatUVW.h>
#include <BBSKernel/MNS/MeqRequest.h>
#include <BBSKernel/MNS/MeqExpr.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>

#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MeasFrame.h>
#include <casa/Quanta/MVuvw.h>
#include <stdint.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

/*
//# Old constructor, uses MeqPhaseRef class which contains a hard-coded
//# reference to the WSRT.

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
  LOG_TRACE_FLOW("Phase ref earth position: " << itsPhaseRef->earthPosition());
}
*/

MeqStatUVW::MeqStatUVW(MeqStation *station, const casa::MDirection &phaseCenter,
    const casa::MPosition &arrayPosition)
    :   itsStation(station),
        itsPhaseCenter(phaseCenter),
        itsArrayPosition(arrayPosition),
  itsU         (0),
  itsV         (0),
  itsW         (0),
  itsLastReqId (InitMeqRequestId)
{
}


void MeqStatUVW::calculate (const MeqRequest& request)
{
  //# Make sure the MeqResult/Matrix objects have the correct size.
  int ntime = request.ny();
  double* uptr = itsU.setDoubleFormat (1, ntime);
  double* vptr = itsV.setDoubleFormat (1, ntime);
  double* wptr = itsW.setDoubleFormat (1, ntime);

  //# Use the UVW coordinates if already calculated for a time.
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
  //# If all found we can exit.
  if (ndone == ntime) {
    itsLastReqId = request.getId();
    return;
  }

  //# Calculate the other UVW coordinates using the AIPS++ code.
  //# First form the time-independent stuff.
  ASSERTSTR (itsStation, "UVW coordinates cannot be calculated");

  //# Get station coordinates in ITRF coordinates
  MeqResult posx = itsStation->getPosX().getResult (request);
  MeqResult posy = itsStation->getPosY().getResult (request);
  MeqResult posz = itsStation->getPosZ().getResult (request);

  LOG_TRACE_FLOW_STR ("posx" << posx.getValue());
  LOG_TRACE_FLOW_STR ("posy" << posy.getValue());
  LOG_TRACE_FLOW_STR ("posz" << posz.getValue());

  //# Get position relative to center to keep values small.
//#  const MVPosition& mvcpos = itsPhaseRef->earthPosition().getValue();
  const MVPosition& mvcpos = itsArrayPosition.getValue();
  MVPosition mvpos(posx.getValue().getDouble() - mvcpos(0),
                   posy.getValue().getDouble() - mvcpos(1),
                   posz.getValue().getDouble() - mvcpos(2));

//  LOG_TRACE_FLOW_STR ("mbl " << mbl);
//#  mbl.getRefPtr()->set(itsFrame);      // attach frame


  for (int i=0; i<ntime; ++i) {
    double time = request.y(i);
//    cout << "Time: " << setprecision(25) << time << endl;
    map<MeqTime,MeqUVW>::iterator pos = itsUVW.find (MeqTime(time));
    if (pos == itsUVW.end()) {

      Quantum<double> qepoch(time, "s");
      MEpoch mepoch(qepoch, MEpoch::UTC);

      MeasFrame frame(itsArrayPosition);
      frame.set(itsPhaseCenter);
      frame.set(mepoch);

//      cout << "Frame: " << setprecision(25) << frame << endl;
//      LOG_TRACE_FLOW_STR ("frame " << mbl.getRefPtr()->getFrame());

      MVBaseline mvbl(mvpos);
//      cout << "mvbl: " << setprecision(25) << mvbl << endl;
      MBaseline mbl(mvbl, MBaseline::ITRF);
//      cout << "mbl: " << setprecision(25) << mbl << endl;
      mbl.getRefPtr()->set(frame);      // attach frame
//      cout << "mbl (after set frame): " << setprecision(25) << mbl << endl;
      MBaseline::Convert mcvt(mbl, MBaseline::J2000);
//      cout << "mcvt: " << setprecision(25) << mcvt << endl;
      const MVBaseline& bas2000 = mcvt().getValue();
//      cout << "bas2000: " << setprecision(25) << bas2000 << endl;

//      LOG_TRACE_FLOW_STR (bas2000);
//#      LOG_TRACE_FLOW_STR (itsPhaseRef->direction().getValue());
//      LOG_TRACE_FLOW_STR (itsPhaseCenter.getValue());
//#      MVuvw uvw2000 (bas2000, itsPhaseRef->direction().getValue());


      MVuvw uvw2000 (bas2000, itsPhaseCenter.getValue());

      const Vector<double>& xyz = uvw2000.getValue();

      LOG_TRACE_FLOW_STR (xyz(0) << ' ' << xyz(1) << ' ' << xyz(2));
      *uptr++ = xyz(0);
      *vptr++ = xyz(1);
      *wptr++ = xyz(2);
/*
      cout << "Baseline: " << hex
<< *reinterpret_cast<unsigned long long int*>(const_cast<double*>(&(xyz(0))))
<< " " << hex
<< *reinterpret_cast<unsigned long long int*>(const_cast<double*>(&(xyz(1))))
<< " " << hex
<< *reinterpret_cast<unsigned long long int*>(const_cast<double*>(&(xyz(2))))
<< dec << endl;
*/
      // Save the UVW coordinates in the map.
      itsUVW[MeqTime(time)] = MeqUVW(xyz(0), xyz(1), xyz(2));
    }
  }
  LOG_TRACE_FLOW_STR ('U' << itsU.getValue());
  LOG_TRACE_FLOW_STR ('V' << itsV.getValue());
  LOG_TRACE_FLOW_STR ('W' << itsW.getValue());

/*
  cout << "Station: " << itsStation->getName() << endl;
  cout << "U: " << setprecision(25) << itsU.getValue() << endl;
  cout << "V: " << setprecision(25) << itsV.getValue() << endl;
  cout << "W: " << setprecision(25) << itsW.getValue() << endl;
*/
  itsLastReqId = request.getId();
}


void MeqStatUVW::set (double time, double u, double v, double w)
{
  itsUVW[MeqTime(time)] = MeqUVW(u,v,w);
}

} // namespace BBS
} // namespace LOFAR
