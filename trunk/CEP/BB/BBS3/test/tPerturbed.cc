//# tPerturbed.cc: Test program to check if perturbed values are correct
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

#include <BBS3/Prediffer.h>
#include <BBS3/Solver.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <Common/VectorUtil.h>
#include <Common/LofarLogger.h>
#include <casa/BasicMath/Math.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <vector>


using namespace LOFAR;
using namespace std;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

void doTest (Prediffer& pre1, const vector<string>& solv)
{
  // Use the prediffer first to calculate the perturbed values.
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>(), true);
  // Set a domain of some channels and all times.
  pre1.setDomain (137750000-250000, 4*500000, 0., 1e12);
  // Get the values of the solvable parms.
  Solver solver;
  solver.initSolvableParmData (1);
  const vector<ParmData>& parmData = pre1.getSolvableParmData();
  solver.setSolvableParmData (parmData, 0);
  vector<double> values = solver.getSolvableValues();
  int nspid = values.size();
  // Get all results.
  vector<MeqResult> results = pre1.getResults();
  // Get the vector of perturbations.
  vector<double> perts(nspid);
  for (int i=0; i<nspid; ++i) {
    perts[i] = 0;
    for (uint j=0; j<results.size(); ++j) {
      if (results[j].isDefined (i)) {
	perts[i] = results[j].getPerturbation(i).getDouble();
	break;
      }
    }
  }
  // Now loop through all solvable parms.
  // Add the perturbation to its value and calculate the result again.
  for (int i=0; i<nspid; ++i) {
    // Add the perturbation for this parm coeff.
    double oldval = values[i];
    values[i] += perts[i];
    pre1.updateSolvableParms (values);
    values[i] = oldval;
    // Calculate without perturbed values.
    vector<MeqResult> res2 = pre1.getResults(false);
    // Now check if values are (about) equal.
    int nrv=0;
    for (uint j=0; j<results.size(); ++j) {
      const MeqMatrix* resm1;
      if (results[j].isDefined (i)) {
	resm1 = &(results[j].getPerturbedValue(i));
      } else {
	resm1 = &(results[j].getValue());
	nrv++;
      }
      const MeqMatrix* resm2 = &(res2[j].getValue());
      ASSERTSTR (resm1->nelements() == resm2->nelements(),
		 "nres1=" << resm1->nelements()
		 << " nres2=" << resm2->nelements()
		 << " valnr=" << j << " nspid=" << i);
      const dcomplex* resv1 = resm1->dcomplexStorage();
      const dcomplex* resv2 = resm2->dcomplexStorage();
      for (int k=0; k<resm1->nelements(); ++k) {
	//	cout <<
	ASSERTSTR (casa::near(real(resv1[k]),real(resv2[k])) &&
		   casa::near(imag(resv1[k]),imag(resv2[k])),
		   "res1=" << resv1[k] << " res2=" << resv2[k] << " rnr=" << k
		   << " valnr=" << j << " spid=" << i);
	//     << endl;
      }
    }
    if (nrv > 0) {
      cout << nrv << " unperturbed values (out of " << results.size()
	   << ") for spid " << i << endl;
    }
  }
}

int main (int argc, const char* argv[])
{
  cout << ">>>" << endl;
  INIT_LOGGER("tPerturbed");
  try {
    if (argc < 7) {
      cerr << "Run as: tPerturbed user msname meqparmtable skyparmtable model solvparms..."
	   << endl;
      return 1;
    }
    // Do a solve using the model and a few stations.
    {
      cout << "Starting first test" << endl;
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 4*i;
      }
      Prediffer pre1(argv[2], argv[3], argv[4], "aips", argv[1], "", "",
		     antVec, argv[5], false, true);
      vector<string> solv(argc-6);
      for (uint i=0; i<solv.size(); ++i) {
	solv[i] = argv[i+6];
      }
      doTest (pre1, solv);
      cout << "End of first test" << endl;
    }
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "<<<" << endl;
  return 0;
}
