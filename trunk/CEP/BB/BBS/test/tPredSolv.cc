//# tPredSolv.cc: Test program for Prediffer and Solver classes
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
#include <BBS/Prediffer.h>
#include <BBS/Solver.h>
#include <BBS/ParmWriter.h>
#include <BBS/MNS/MeqParmFunklet.h>
#include <ParmDB/ParmDB.h>
#include <Common/VectorUtil.h>
#include <Common/LofarLogger.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <malloc.h>

#include <Common/hexdump.h>


using namespace LOFAR;
using namespace std;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

void doSolve (Prediffer& pre1, const vector<string>& solv, int niter)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>());
  // Set a domain.
  cout << ">>>" << endl;
  pre1.setWorkDomain (0, 1, 0., 1e12);
  //  pre1.setWorkDomain (137750000-250000, 2*500000, 0., 1e12);
  //  pre1.setWorkDomain (0., 1e12, 0., 1e12);
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  pre1.initSolvableParms (solveDomains);
  cout << "<<<" << endl;
  // Get the ParmData from the Prediffer and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (1, solveDomains, pre1.getWorkDomain());
  solver.setSolvableParmData (pre1.getSolvableParmData(), 0);
  cout << ">>>" << endl;
  pre1.showSettings();
  cout << "<<<" << endl;
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(10) << solver.getSolvableValues(0)
       << endl;

  for (int it=0; it<niter; ++it) {
    // Get the fitter from the prediffer and give it to the solver.
    vector<casa::LSQFit> fitters;
    cout << ">>>" << endl;
    pre1.fillFitters (fitters, "DATA");
    solver.mergeFitters (fitters, 0);
    // Do the solve.
    solver.solve (false);
    cout << "<<<" << endl;
    cout << "iter" << it << ":  " << setprecision(10)
	 << solver.getSolvableValues(0) << endl;
    cout << solver.getQuality(0) << endl;
    cout.precision (prec);
    pre1.updateSolvableParms (solver.getSolvableParmData());
  }
}

void doSolve2 (Prediffer& pre1, Prediffer& pre2,
	       const vector<string>& solv, int niter)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre2.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>());
  pre2.setSolvableParms (solv, vector<string>());
  // Set a domain.
  cout << ">>>" << endl;
  pre1.setWorkDomain (0, 3, 0., 1e12);
  pre2.setWorkDomain (0, 3, 0., 1e12);
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  pre1.initSolvableParms (solveDomains);
  pre2.initSolvableParms (solveDomains);
  cout << "<<<" << endl;
  // Get the ParmData from the Prediffers and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (2, solveDomains, pre1.getWorkDomain());
  solver.setSolvableParmData (pre1.getSolvableParmData(), 0);
  solver.setSolvableParmData (pre2.getSolvableParmData(), 1);
    
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(10) << solver.getSolvableValues(0)
       << endl;
  for (int it=0; it<niter; ++it) {
    // Get the fitter from the prediffer and give it to the solver.
    vector<casa::LSQFit> fitters;
    cout << ">>>" << endl;
    pre1.fillFitters (fitters, "DATA");
    solver.mergeFitters (fitters, 0);
    pre2.fillFitters (fitters, "DATA");
    solver.mergeFitters (fitters, 1);
    // Do the solve.
    solver.solve (false);
    cout << "<<<" << endl;
    cout << "iter" << it << ":  " << setprecision(10)
	 << solver.getSolvableValues(0) << endl;
    cout << solver.getQuality(0) << endl;
    cout.precision (prec);
    pre1.updateSolvableParms (solver.getSolvableParmData());
    pre2.updateSolvableParms (solver.getSolvableParmData());
  }
}

void doSolve1 (Prediffer& pre1, const vector<string>& solv, int niter)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>());
  // Set a domain.
  cout << ">>>" << endl;
  pre1.setWorkDomain (0, 3, 0., 1e12);
  //pre1.setWorkDomain (0., 1e12, 0., 1e12);
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  pre1.initSolvableParms (solveDomains);
  cout << "<<<" << endl;
  // Get the ParmData from the Prediffer and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (1, solveDomains, pre1.getWorkDomain());
  const ParmDataInfo& pData = pre1.getSolvableParmData();
  solver.setSolvableParmData (pData, 0);
  cout << ">>>" << endl;
  pre1.showSettings();
  cout << "<<<" << endl;
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(10) << solver.getSolvableValues(0)
       << endl;

  for (int it=0; it<niter; ++it) {
    // Get the fitter from the prediffer and give it to the solver.
    vector<casa::LSQFit> fitters;
    cout << ">>>" << endl;
    pre1.fillFitters (fitters, "DATA");
    solver.mergeFitters (fitters, 0);
    // Do the solve.
    solver.solve (false);
    cout << "<<<" << endl;
    cout << "iter" << it << ":  " << setprecision(10)
	 << solver.getSolvableValues(0) << endl;
    cout << solver.getQuality(0) << endl;
    cout.precision (prec);
    MeqDomain domain(pre1.getWorkDomain());
    ParmWriter pwriter;
    pwriter.write (solver.getSolvableParmData(),
		   domain.startX(), domain.endX(),
		   domain.startY(), domain.endY());
    pre1.updateSolvableParms();
  }
}


int main (int argc, const char* argv[])
{
  INIT_LOGGER("tPredSolv");
  try {
    if (argc < 5) {
      cerr << "Run as: tPredSolv user msname meqparmtable skyparmtable"
	   << endl;
      return 1;
    }

    // Read the info for the ParmTables
    ParmDB::ParmDBMeta meqPdm("aips", argv[3]);
    ParmDB::ParmDBMeta skyPdm("aips", argv[4]);

    // Do a solve for RA using a few stations.
    {
      cout << "Starting first test" << endl;
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      vector<vector<int> > srcgrp;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 
		     antVec, "TOTALEJ.REALIMAG", srcgrp, false);
      // Do a further selection of a few stations.
      vector<int> antVec2(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      vector<int> corr;
      pre1.select (antVec2, antVec2, false, corr);    // no autocorrelations
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve (pre1, solv, 9);
      cout << "End of first test" << endl;
    }
    // Do a solve using 2 prediffers.
    {
      cout << "Starting test with two prediffers" << endl;
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      vector<vector<int> > srcgrp;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 
		     antVec, "TOTALEJ.REALIMAG", srcgrp, false);
      Prediffer pre2(argv[2], meqPdm, skyPdm, 
		     antVec, "TOTALEJ.REALIMAG", srcgrp, false);
      // Do a further selection of a few stations.
      vector<int> antVec2(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      vector<int> corr;
      pre1.select (antVec2, antVec2, false, corr);    // no autocorrelations
      pre2.select (antVec2, antVec2, false, corr);    // no autocorrelations
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve2 (pre1, pre2, solv, 1);
      cout << "End of test with two prediffers" << endl;
    }
    // Take more baselines.
    {
      cout << "Starting test with 21 antennas" << endl;
      vector<int> antVec(21);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 4*i;
      }
      vector<vector<int> > srcgrp;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 
		     antVec, "TOTALEJ.REALIMAG", srcgrp, false);
      // Only use first correlation.
      vector<int> corrVec(1, 0);
      vector<int> antVec2;
      pre1.select (antVec2, antVec2, false, corrVec);    // no autocorrelations
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve (pre1, solv, 5);
      cout << "End of test with 21 antennas" << endl;
    }
    // Do a solve updating the parm table.
    // This should be the last one.
    {
      cout << "Starting test with updating parmtable" << endl;
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      vector<vector<int> > srcgrp;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 
		     antVec, "TOTALEJ.REALIMAG", srcgrp, false);
      // Do a further selection of a few stations.
      vector<int> antVec2(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      vector<int> corr;
      pre1.select (antVec2, antVec2, false, corr);    // no autocorrelations
      vector<string> solv(3);
      solv[0] = "RA.*";
      solv[1] = "DEC.*";
      solv[2] = "StokesI.*";
      doSolve1 (pre1, solv, 5);
      cout << "End of test with updating parmtable" << endl;
    }

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
