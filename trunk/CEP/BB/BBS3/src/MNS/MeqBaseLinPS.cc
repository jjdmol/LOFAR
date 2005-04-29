//# MeqBaseLinPS.cc: Baseline prediction of a point source with linear polarisation
//#
//# Copyright (C) 2005
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
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqBaseLinPS.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR {

MeqBaseLinPS::MeqBaseLinPS (const MeqExpr& dft, MeqPointSource* source)
: itsDFT    (dft),
  itsSource (source)
{}

MeqBaseLinPS::~MeqBaseLinPS()
{}

MeqJonesResult MeqBaseLinPS::getResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  // We can only calculate for a single time bin.
  ASSERT (request.ny() == 1);
  // Allocate the result.
  MeqJonesResult result(request.nspid());
  MeqResult& resXX = result.result11();
  MeqResult& resXY = result.result12();
  MeqResult& resYX = result.result21();
  MeqResult& resYY = result.result22();
  // Calculate the source fluxes.
  MeqResult ik = itsSource->getI().getResult (request);
  MeqResult qk = itsSource->getQ().getResult (request);
  MeqResult uk = itsSource->getU().getResult (request);
  MeqResult vk = itsSource->getV().getResult (request);
  // Calculate the baseline DFT.
  MeqResult dft = itsDFT.getResult (request);
  // Calculate the XX values, etc.
  MeqMatrix uvk = tocomplex(uk.getValue(), vk.getValue());
  resXX.setValue ((ik.getValue() + qk.getValue()) * dft.getValue());
  resXY.setValue (uvk * dft.getValue());
  resYX.setValue (conj(uvk) * dft.getValue());
  resYY.setValue ((ik.getValue() - qk.getValue()) * dft.getValue());

  // Evaluate (if needed) for the perturbed parameter values.
  MeqMatrix perturbation;
  for (int spinx=0; spinx<request.nspid(); spinx++) {
    bool eval1 = false;
    bool eval2 = false;
    if (dft.isDefined(spinx)) {
      eval1 = true;
      eval2 = true;
      perturbation = dft.getPerturbation (spinx);
    } else {
      if (ik.isDefined(spinx)) {
	eval1 = true;
	perturbation = ik.getPerturbation (spinx);
      } else if (qk.isDefined(spinx)) {
	eval1 = true;
	perturbation = qk.getPerturbation (spinx);
      }
      if (uk.isDefined(spinx)) {
	eval2 = true;
	perturbation = uk.getPerturbation (spinx);
      } else if (vk.isDefined(spinx)) {
	eval2 = true;
	perturbation = vk.getPerturbation (spinx);
      }
    }
    if (eval1) {
      const MeqMatrix& ikp = ik.getPerturbedValue(spinx);
      const MeqMatrix& qkp = qk.getPerturbedValue(spinx);
      resXX.setPerturbedValue (spinx,
			       (ikp+qkp) * dft.getPerturbedValue(spinx));
      resYY.setPerturbedValue (spinx,
			       (ikp-qkp) * dft.getPerturbedValue(spinx));
      resXX.setPerturbation (spinx, perturbation);
      resYY.setPerturbation (spinx, perturbation);
    }
    if (eval2) {
      MeqMatrix uvk = tocomplex(uk.getPerturbedValue(spinx),
				vk.getPerturbedValue(spinx));
      resXY.setPerturbedValue (spinx,
			       uvk * dft.getPerturbedValue(spinx));
      resYX.setPerturbedValue (spinx,
			       conj(uvk) * dft.getPerturbedValue(spinx));
      resXY.setPerturbation (spinx, perturbation);
      resYX.setPerturbation (spinx, perturbation);
    }
  }
  return result;
}

}
