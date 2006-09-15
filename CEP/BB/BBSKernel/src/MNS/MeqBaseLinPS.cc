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
//#include <Common/Timer.h>

#include <BBSKernel/MNS/MeqBaseLinPS.h>
#include <BBSKernel/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

MeqBaseLinPS::MeqBaseLinPS (const MeqExpr& dft, MeqPointSource* source)
: itsDFT    (dft),
  itsSource (source)
{
  addChild (itsDFT);
  addChild (itsSource->getI());
  addChild (itsSource->getQ());
  addChild (itsSource->getU());
  addChild (itsSource->getV());
}

MeqBaseLinPS::~MeqBaseLinPS()
{}

MeqJonesResult MeqBaseLinPS::getJResult (const MeqRequest& request)
{
  //static NSTimer timer("MeqBaseLinPS::getResult", true);
  //timer.start();

  // Allocate the result.
  MeqJonesResult result(request.nspid());
  {
    MeqResult& resXX = result.result11();
    MeqResult& resXY = result.result12();
    MeqResult& resYX = result.result21();
    MeqResult& resYY = result.result22();
    // Calculate the source fluxes.
    MeqResult ikBuf, qkBuf, ukBuf, vkBuf;
    const MeqResult& ik = itsSource->getI().getResultSynced (request, ikBuf);
    const MeqResult& qk = itsSource->getQ().getResultSynced (request, qkBuf);
    const MeqResult& uk = itsSource->getU().getResultSynced (request, ukBuf);
    const MeqResult& vk = itsSource->getV().getResultSynced (request, vkBuf);
    // Calculate the baseline DFT.
    MeqResult dftBuf;
    const MeqResult& dft = itsDFT.getResultSynced (request, dftBuf);
    // Calculate the XX values, etc.
    MeqMatrix uvk = tocomplex(uk.getValue(), vk.getValue());
    resXX.setValue ((ik.getValue() + qk.getValue()) * dft.getValue());
    resXY.setValue (uvk * dft.getValue());
    resYX.setValue (conj(uvk) * dft.getValue());
    resYY.setValue ((ik.getValue() - qk.getValue()) * dft.getValue());
    // Evaluate (if needed) for the perturbed parameter values.
    const MeqParmFunklet* perturbedParm;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      bool eval1 = false;
      bool eval2 = false;
      if (dft.isDefined(spinx)) {
    eval1 = true;
    eval2 = true;
    perturbedParm = dft.getPerturbedParm (spinx);
      } else {
    if (ik.isDefined(spinx)) {
      eval1 = true;
      perturbedParm = ik.getPerturbedParm (spinx);
    } else if (qk.isDefined(spinx)) {
      eval1 = true;
      perturbedParm = qk.getPerturbedParm (spinx);
    }
    if (uk.isDefined(spinx)) {
      eval2 = true;
      perturbedParm = uk.getPerturbedParm (spinx);
    } else if (vk.isDefined(spinx)) {
      eval2 = true;
      perturbedParm = vk.getPerturbedParm (spinx);
    }
      }
      if (eval1) {
    const MeqMatrix& ikp = ik.getPerturbedValue(spinx);
    const MeqMatrix& qkp = qk.getPerturbedValue(spinx);
    resXX.setPerturbedValue (spinx,
                 (ikp+qkp) * dft.getPerturbedValue(spinx));
    resYY.setPerturbedValue (spinx,
                 (ikp-qkp) * dft.getPerturbedValue(spinx));
    resXX.setPerturbedParm (spinx, perturbedParm);
    resYY.setPerturbedParm (spinx, perturbedParm);
      }
      if (eval2) {
    MeqMatrix uvk = tocomplex(uk.getPerturbedValue(spinx),
                  vk.getPerturbedValue(spinx));
    resXY.setPerturbedValue (spinx,
                 uvk * dft.getPerturbedValue(spinx));
    resYX.setPerturbedValue (spinx,
                 conj(uvk) * dft.getPerturbedValue(spinx));
    resXY.setPerturbedParm (spinx, perturbedParm);
    resYX.setPerturbedParm (spinx, perturbedParm);
      }
    }
  }
  //timer.stop();
  return result;
}

#ifdef EXPR_GRAPH
std::string MeqBaseLinPS::getLabel()
{
    return std::string("MeqBaseLinPS\\nPrediction of a linearly polarized point source\\n" + itsSource->getName() + " (" + itsSource->getGroupName() + ")");
}
#endif

} // namespace BBS
} // namespace LOFAR
