//# MeqLofarStatSource.cc: The Jones expression for a station
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

#include <PSS3/MNS/MeqLofarStatSources.h>
#include <PSS3/MNS/MeqRequest.h>
#include <PSS3/MNS/MeqMatrix.h>
#include <PSS3/MNS/MeqMatrixTmp.h>
#include <Common/Debug.h>

MeqLofarStatSources::MeqLofarStatSources (const vector<MeqJonesExpr*>& stat,
					  MeqStatSources* sources)
: itsStat (stat),
  itsSrc  (sources),
  itsLastReqId (InitMeqRequestId)
{
  Assert (stat.size() == sources->nsources());
}

MeqLofarStatSources::~MeqLofarStatSources()
{}

void MeqLofarStatSources::calcResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  Assert (request.nx() == 1);
  MeqRequest req(request);
  // Size the K vector such that all channels can be held.
  int nfreq = request.ny();
  complex<double>* kdat = itsK.setDComplex (1, nfreq);
  // Multiply the station J matrices by the source contribution K.
  int nrsrc = itsSrc->nsources();
  for (int i=0; i<nrsrc; i++) {
    req.setSourceNr (i);
    int srcnr = itsSrc->actualSourceNr(i);
    MeqJonesExpr* jjones = itsStat[srcnr];
    const MeqResult& kjones = itsSrc->getResult (req);
    const MeqResult* kdel = 0;
    // Get the value of the first channel (exp(2.pi.i(ul+vm+wn))/sqrt(n)
    // See also MeqStatSources for more info.
    // Note that a baseline AB has uvw coordinates u(B) - u(A).
    // Therefore the conjugate is applied to the J Jones.
    complex<double> val0 = conj(kjones.getValue().getDComplex());
    kdat[0] = val0;
    if (nfreq > 1) {
      // The next channels are times a constant factor.
      kdel = &(itsSrc->getDelta (req));
      complex<double> delta = conj(kdel->getValue().getDComplex());
      for (int i=1; i<nfreq; i++) {
	val0 *= delta;
	kdat[i] = val0;
      }
    }
    // Get the J Jones matrices.
    MeqResult& j11 = jjones->result11 (request);
    MeqResult& j12 = jjones->result12 (request);
    MeqResult& j21 = jjones->result21 (request);
    MeqResult& j22 = jjones->result22 (request);
    // The result of J*K is kept in J.
    // The same is true for perturbed values.
    // So if K has a perturbed value for a parameter and J has not,
    // the value of J has to be copied first to make it perturbed.
    if (kjones.nperturbed() > 0) {
      for (int j=0; j<request.nspid(); j++) {
	if (kjones.isDefined(j)) {
	  const MeqMatrix& perturbation = kjones.getPerturbation(j);
	  if (! j11.isDefined(j)) {
	    j11.setPerturbedValue (j, j11.getValue().clone());
	    j11.setPerturbation (j, perturbation);
	  }
	  if (! j12.isDefined(j)) {
	    j12.setPerturbedValue (j, j12.getValue().clone());
	    j12.setPerturbation (j, perturbation);
	  }
	  if (! j21.isDefined(j)) {
	    j21.setPerturbedValue (j, j21.getValue().clone());
	    j21.setPerturbation (j, perturbation);
	  }
	  if (! j22.isDefined(j)) {
	    j22.setPerturbedValue (j, j22.getValue().clone());
	    j22.setPerturbation (j, perturbation);
	  }
	}
      }
    }
    // Now multiple the J Jones with K and keep the result in J.
    {
      MeqMatrix& val = j11.getValueRW();
      if (val.nelements() == nfreq  &&  !val.isDouble()) {
	val *= itsK;
      } else {
	val = val * itsK;
      }
    }
    {
      MeqMatrix& val = j12.getValueRW();
      if (val.nelements() == nfreq  &&  !val.isDouble()) {
	val *= itsK;
      } else {
	val = val * itsK;
      }
    }
    {
      MeqMatrix& val = j21.getValueRW();
      if (val.nelements() == nfreq  &&  !val.isDouble()) {
	val *= itsK;
      } else {
	val = val * itsK;
      }
    }
    {
      MeqMatrix& val = j22.getValueRW();
      if (val.nelements() == nfreq  &&  !val.isDouble()) {
	val *= itsK;
      } else {
	val = val * itsK;
      }
    }
//     cout << "j11.getValue before: ";
//     j11.getValue().show(cout);
//     cout << endl;
    // Do the same for the perturbed values.
    MeqMatrix perturbation;
    for (int j=0; j<request.nspid(); j++) {
      MeqMatrix* kpmat = &itsK;
      bool eval = false;
      if (kjones.isDefined(j)) {
	kpmat = &itsPertK;
	eval = true;
	perturbation = kjones.getPerturbation(j);
	complex<double>* kdatp = itsPertK.setDComplex (1, nfreq);
	val0 = conj(kjones.getPerturbedValue(j).getDComplex());
	kdatp[0] = val0;
	if (nfreq > 1) {
	  complex<double> delta = conj(kdel->getPerturbedValue(j).getDComplex());
	  for (int i=1; i<nfreq; i++) {
	    val0 *= delta;
	    kdatp[i] = val0;
	  }
	}
      }
      if (eval || j11.isDefined(j)) {
	MeqMatrix& val = j11.getPerturbedValueRW(j);
	if (val.nelements() == nfreq  &&  !val.isDouble()) {
	  val *= *kpmat;
	} else {
	  val = val * *kpmat;
	}
      }
      if (eval || j12.isDefined(j)) {
	MeqMatrix& val = j12.getPerturbedValueRW(j);
	if (val.nelements() == nfreq  &&  !val.isDouble()) {
	  val *= *kpmat;
	} else {
	  val = val * *kpmat;
	}
      }
      if (eval || j21.isDefined(j)) {
	MeqMatrix& val = j21.getPerturbedValueRW(j);
	if (val.nelements() == nfreq  &&  !val.isDouble()) {
	  val *= *kpmat;
	} else {
	  val = val * *kpmat;
	}
      }
      if (eval || j22.isDefined(j)) {
	MeqMatrix& val = j22.getPerturbedValueRW(j);
	if (val.nelements() == nfreq  &&  !val.isDouble()) {
	  val *= *kpmat;
	} else {
	  val = val * *kpmat;
	}
      }
//       cout << "LSS " << j << ' ' << j11.getValue() << ' '
//  	   << j11.getPerturbedValue(j) << endl
// 	   << "     " << j12.getValue()
//  	   << j12.getPerturbedValue(j) << endl
// 	   << "     " << j21.getValue()
//  	   << j21.getPerturbedValue(j) << endl
// 	   << "     " << j22.getValue()
//  	   << j22.getPerturbedValue(j) << endl
//  	   << endl;
    }
//     cout << "j11.getValue after: ";
//     j11.getValue().show(cout);
//     cout << endl;
  }
  itsLastReqId = request.getId();
}
