//# MeqWsrtPoint.cc: The total baseline prediction of point sources in the WSRT model
//#
//# Copyright (C) 2002,2003
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

#include <BBS3/MNS/MeqWsrtPoint.h>
#include <BBS3/MNS/MeqPointDFT.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqHist.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR {

MeqWsrtPoint::MeqWsrtPoint (MeqSourceList* sources,
			    MeqPointDFT* dft,
			    MeqHist* celltHistogram, MeqHist* cellfHistogram)
: itsSources   (sources),
  itsDFT       (dft),
  itsCelltHist (celltHistogram),
  itsCellfHist (cellfHistogram)
{}

MeqWsrtPoint::~MeqWsrtPoint()
{}

void MeqWsrtPoint::calcResult (const MeqRequest& request)
{
  PERFPROFILE(__PRETTY_FUNCTION__);

  // We can only calculate for a single time bin.
  const MeqDomain& domain = request.domain();
  ASSERT (request.ny() == 1);
  // Find the maximum nr of cells needed.
  int ncellt = 1;
  int ncellf = 1;
  int nrsrc = itsSources->size();
/*
  for (int i=0; i<nrsrc; i++) {
    vector<int> ncell = itsDFT->ncells (i, request);
    if (ncell[0] > ncellt) {
      ncellt = ncell[0];
    }
    if (ncell[1] > ncellf) {
      ncellf = ncell[1];
    }
  }
*/
  itsNcell.resize (2);
  itsNcell[0] = ncellt;
  itsNcell[1] = ncellf;
  ///  if (ncellf > 10) {
    ///    cout << "ncellf=" << ncellf << endl;
    ///  }
  
  itsCelltHist->update (ncellt);
  itsCellfHist->update (ncellf);
  ncellf *= request.nx();
  // The domain is divided into the required number of cells.
  MeqRequest dftReq (domain, ncellf, ncellt, request.nspid());
  MeqResult& resXX = result11();
  MeqResult& resXY = result12();
  MeqResult& resYX = result21();
  MeqResult& resYY = result22();
  resXX = MeqResult(request.nspid());
  resXY = MeqResult(request.nspid());
  resYX = MeqResult(request.nspid());
  resYY = MeqResult(request.nspid());
  Matrix<dcomplex> value(ncellt, ncellf, makedcomplex(0,0));
  resXX.setValue (MeqMatrix (value));
  resXY.setValue (MeqMatrix (value));
  resYX.setValue (MeqMatrix (value));
  resYY.setValue (MeqMatrix (value));
  
  for (int srcnr=0; srcnr<nrsrc; srcnr++) {
    dftReq.setSourceNr (srcnr);
    MeqPointSource& src = (*itsSources)[srcnr];
    MeqResult ik = src.getI()->getResult (dftReq);
    MeqResult qk = src.getQ()->getResult (dftReq);
    MeqResult uk = src.getU()->getResult (dftReq);
    MeqResult vk = src.getV()->getResult (dftReq);
    MeqResult dft = itsDFT->getResult (dftReq);
    MeqMatrix vki = tocomplex(0., vk.getValue());
    if (MeqPointDFT::doshow) {
      cout << "MeqWsrtPoint iquvk,dft: " << ik.getValue() << ' '
	   << qk.getValue() << ' ' << uk.getValue() << ' ' << vk.getValue()
	   << ' ' << vki << ' ' << dft.getValue() << endl;
    }
    // Calculate XX, etc.
    MeqMatrix xx = (ik.getValue() + qk.getValue()) * 0.5 * dft.getValue();
    if (MeqPointDFT::doshow) {
      cout << "MeqWsrtPoint abs(xx) " << abs(xx.getDComplex(0,0)) << endl;
    }
    MeqMatrix xy = (uk.getValue() + vki) * 0.5 * dft.getValue();
    MeqMatrix yx = (uk.getValue() - vki) * 0.5 * dft.getValue();
    MeqMatrix yy = (ik.getValue() - qk.getValue()) * 0.5 * dft.getValue();
    if (MeqPointDFT::doshow) {
      cout << "MeqWsrtPoint XX: " << xx << endl << resXX.getValue() << endl;
    }

    // Evaluate (if needed) for the perturbed parameter values.
    MeqMatrix perturbation;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      // Determine which expression part is perturbed.
      bool evaliq = false;
      if (ik.isDefined(spinx)) {
	evaliq = true;
	perturbation = ik.getPerturbation(spinx);
      }
      if (qk.isDefined(spinx)) {
	evaliq = true;
	perturbation = qk.getPerturbation(spinx);
      }
      bool evaluv = false;
      if (uk.isDefined(spinx)) {
	evaluv = true;
	perturbation = uk.getPerturbation(spinx);
      }
      MeqMatrix vkip(vki);
      if (vk.isDefined(spinx)) {
	evaluv = true;
	perturbation = vk.getPerturbation(spinx);
	vkip = tocomplex(0., vk.getPerturbedValue(spinx));
      }
      if (dft.isDefined(spinx)) {
	evaliq = true;
	evaluv = true;
	perturbation = dft.getPerturbation(spinx);
      }
      // Calculate if one of the results is perturbed for this parameter.
      if (evaliq || evaluv) {
	const MeqMatrix& dkp = dft.getPerturbedValue(spinx);
	if (evaliq) {
	  const MeqMatrix& ikp = ik.getPerturbedValue(spinx);
	  const MeqMatrix& qkp = qk.getPerturbedValue(spinx);
	  MeqMatrix xxp = (ikp + qkp) * 0.5 * dkp;
	  MeqMatrix yyp = (ikp - qkp) * 0.5 * dkp;
	  // If not calculated before, initialize to unperturbed sum.
	  if (! resXX.isDefined(spinx)) {
	    resXX.setPerturbedValue (spinx, resXX.getValue().clone());
	    resXX.setPerturbation (spinx, perturbation);
	  }
	  if (! resYY.isDefined(spinx)) {
	    resYY.setPerturbedValue (spinx, resYY.getValue().clone());
	    resYY.setPerturbation (spinx, perturbation);
	  }
	  resXX.getPerturbedValueRW(spinx) += xxp;
	  resYY.getPerturbedValueRW(spinx) += yyp;
	}
	if (evaluv) {
	  const MeqMatrix& ukp = uk.getPerturbedValue(spinx);
	  MeqMatrix xyp = (ukp + vkip) * 0.5 * dkp;
	  MeqMatrix yxp = (ukp - vkip) * 0.5 * dkp;
	  if (! resXY.isDefined(spinx)) {
	    resXY.setPerturbedValue (spinx, resXY.getValue().clone());
	    resXY.setPerturbation (spinx, perturbation);
	  }
	  if (! resYX.isDefined(spinx)) {
	    resYX.setPerturbedValue (spinx, resYX.getValue().clone());
	    resYX.setPerturbation (spinx, perturbation);
	  }
	  resXY.getPerturbedValueRW(spinx) += xyp;
	  resYX.getPerturbedValueRW(spinx) += yxp;
	}
      } else {
	// No perturbed values in result for this parameter.
	// Add unperturbed value if previous results were perturbed.
	if (resXX.isDefined(spinx)) {
	  resXX.getPerturbedValueRW(spinx) += xx;
	}
	if (resXY.isDefined(spinx)) {
	  resXY.getPerturbedValueRW(spinx) += xy;
	}
	if (resYX.isDefined(spinx)) {
	  resYX.getPerturbedValueRW(spinx) += yx;
	}
	if (resYY.isDefined(spinx)) {
	  resYY.getPerturbedValueRW(spinx) += yy;
	}
      }
    }

    // Now add the source contribution to the unperturbed value.
    resXX.getValueRW() += xx;
    resXY.getValueRW() += xy;
    resYX.getValueRW() += yx;
    resYY.getValueRW() += yy;
  }
}

}
