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

#include <Common/Profiling/PerfProfile.h>

#include <MNS/MeqStatUVW.h>
#include <MNS/MeqStation.h>
#include <MNS/MeqPhaseRef.h>
#include <MNS/MeqRequest.h>
#include <MNS/MeqExpr.h>
#include <MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>
#include <aips/Measures/MBaseline.h>
#include <aips/Measures/MPosition.h>
#include <aips/Measures/MEpoch.h>
#include <aips/Measures/MeasConvert.h>
#include <aips/Quanta/MVuvw.h>


MeqStatUVW::MeqStatUVW (MeqStation* station,
			const MeqPhaseRef* phaseRef)
: itsStation   (station),
  itsPhaseRef  (phaseRef),
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
  PERFPROFILE(__PRETTY_FUNCTION__);
  // Make sure the MeqResult/Matrix objects have the correct size.
  double* uptr = itsU.setDouble (request.nx(), 1);
  double* vptr = itsV.setDouble (request.nx(), 1);
  double* wptr = itsW.setDouble (request.nx(), 1);
  double step = request.stepX();
  double time = request.domain().startX() + step/2;
  // Use the UVW coordinates if already calculated for this time.
  MeqTime meqtime(time);
  if (request.nx() == 1) {
    map<MeqTime,MeqUVW>::iterator pos = itsUVW.find (meqtime);
    if (pos != itsUVW.end()) {
      uptr[0] = pos->second.itsU;
      vptr[0] = pos->second.itsV;
      wptr[0] = pos->second.itsW;
      itsLastReqId = request.getId();
      return;
    }
  }
  // Calculate the UVW coordinates using the AIPS++ code.
  AssertStr (itsStation, "UVW coordinates cannot be calculated");
  MeqResult posx = itsStation->getPosX()->getResult (request);
  MeqResult posy = itsStation->getPosY()->getResult (request);
  MeqResult posz = itsStation->getPosZ()->getResult (request);
  TRACER1 ("posx" << posx.getValue());
  TRACER1 ("posy" << posy.getValue());
  TRACER1 ("posz" << posz.getValue());
  MVPosition mvpos(posx.getValue().getDouble(),
		   posy.getValue().getDouble(),
		   posz.getValue().getDouble());
  MVBaseline mvbl(mvpos);
  MBaseline mbl(mvbl, MBaseline::ITRF);
  TRACER1 ("mbl " << mbl);
  Quantum<double> qepoch(0, "s");
  qepoch.setValue (time);
  MEpoch mepoch(qepoch, MEpoch::UTC);
  itsFrame.set (mepoch);
  mbl.getRefPtr()->set(itsFrame);      // attach frame
  MBaseline::Convert mcvt(mbl, MBaseline::J2000);
  for (Int i=0; i<request.nx(); i++) {
    itsFrame.set (mepoch);
    TRACER1 ("frame " << mbl.getRefPtr()->getFrame());
    const MVBaseline& bas2000 = mcvt().getValue();
    TRACER1 (bas2000);
    TRACER1 (itsPhaseRef->direction().getValue());
    MVuvw uvw2000 (bas2000, itsPhaseRef->direction().getValue());
    const Vector<double>& xyz = uvw2000.getValue();
    TRACER1 (xyz(0) << ' ' << xyz(1) << ' ' << xyz(2));
    *uptr++ = xyz(0);
    *vptr++ = xyz(1);
    *wptr++ = xyz(2);
    // Save the UVW coordinates in the map.
    itsUVW[MeqTime(time)] = MeqUVW(xyz(0), xyz(1), xyz(2));
    // Go to next time step.
    time += step;
    qepoch.setValue (time);
    mepoch.set (qepoch);
  }

//   double hav = itsPhaseRef->getStartHA() +
//                (request.domain().startX() + request.stepX()/2 -
//                 itsPhaseRef->getStartTime()) * itsPhaseRef->getScaleHA();
//   double haStep = request.stepX() * itsPhaseRef->getScaleHA();
//   // Make a double matrix for the hourangle values.
//   MeqMatrix ha(0., request.nx(), 1, false);
//   double* haval = ha.doubleStorage();
//   haval[0] = hav;
//   for (int i=1; i<request.nx(); i++) {
//     hav += haStep;
//     haval[i] = hav;
//   }
//     cout << "ha" << ha << endl;
//     cout << "ra,dec " << itsPhaseRef->getRa() << ' '
//        << itsPhaseRef->getDec() << endl;
//   MeqMatrix sinha = sin(ha);
//   MeqMatrix cosha = cos(ha);
//   MeqMatrix sindec(itsPhaseRef->getSinDec());
//   MeqMatrix cosdec(itsPhaseRef->getCosDec());
//   // Make HA the last factors, because usually the others are scalars.
//   // Normal evaluation is left to right, so they result in a scalar
//   // multiplication.
//   itsU.setValue (posx.getValue() * sinha + posy.getValue() * cosha);
//   itsV.setValue (sindec * posy.getValue() * sinha - 
// 		 sindec * posx.getValue() * cosha +
// 		 cosdec * posz.getValue());
//   itsW.setValue (cosdec * posx.getValue() * cosha - 
// 		 cosdec * posy.getValue() * sinha +
// 		 sindec * posz.getValue());
  TRACER1 ('U' << itsU.getValue());
  TRACER1 ('V' << itsV.getValue());
  TRACER1 ('W' << itsW.getValue());

  // Evaluate (if needed) for the perturbed parameter values.
  // Only station positions can be perturbed.
//   MeqMatrix perturbation;
//   for (int spinx=0; spinx<request.nspid(); spinx++) {
//     bool eval = false;
//     bool evalu = false;
//     if (posx.isDefined(spinx)) {
//       perturbation = posx.getPerturbation(spinx);
//       evalu = true;
//       eval  = true;
//     }
//     if (posy.isDefined(spinx)) {
//       perturbation = posy.getPerturbation(spinx);
//       evalu = true;
//       eval  = true;
//     }
//     if (posz.isDefined(spinx)) {
//       perturbation = posz.getPerturbation(spinx);
//       eval  = true;
//     }
//     if (eval) {
//       if (evalu) {
// 	itsU.setPerturbedValue (spinx,
// 				posx.getPerturbedValue(spinx) * sinha +
// 				posy.getPerturbedValue(spinx) * cosha);
// 	itsU.setPerturbation (spinx, perturbation);
//       }
//       itsV.setPerturbedValue (spinx,
// 			      sindec * posy.getPerturbedValue(spinx) * sinha - 
// 			      cosdec * posx.getPerturbedValue(spinx) * cosha +
// 			      cosdec * posz.getPerturbedValue(spinx));
//       itsV.setPerturbation (spinx, perturbation);
//       itsW.setPerturbedValue (spinx,
// 			      cosdec * posx.getPerturbedValue(spinx) * cosha - 
// 			      cosdec * posy.getPerturbedValue(spinx) * sinha +
// 			      sindec * posz.getPerturbedValue(spinx));
//       itsW.setPerturbation (spinx, perturbation);
//     }
//   }
  itsLastReqId = request.getId();
}

void MeqStatUVW::set (double time, double u, double v, double w)
{
  itsUVW[MeqTime(time)] = MeqUVW(u,v,w);
}
