//# t3C343.cc: Program for 3C343 calibration
//#
//# Copyright (C) 2005
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <BBS/Prediffer.h>
#include <BBS/Solver.h>
#include <BBS/MNS/MeqParmFunklet.h>
#include <BBS/BBSTestLogger.h>
#include <ParmDB/ParmDB.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <malloc.h>

using namespace LOFAR;
using namespace std;


// Note:
// 3C343/10008336.MS contains 64 frequency channels of 156250 Hz with
// center frequencies of 1.18-1.17015625 GHz.
// There are 1437 time slots of 30 sec in it, but the first one is 20 seconds.
// timeStart is 4.47203e+09-4260-10 and timeLast is timeStart+43075+25

// Solve for the entire time domain.
void doSolveAll (Prediffer& pre1, const vector<string>& solv,
		 int maxniter, double epsilon=1e-4)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, vector<string>());
  // Set a domain. Only use center frequencies and all times.
  //pre1.setDomain (1170078125+24*156250, 16*156250, 0., 1e12);
  pre1.setWorkDomain (4, 59, 0., 1e12);
  //pre1.setWorkDomain (1170078125+34*156250, 4*156250, 0., 1e12);
  ///pre1.setWorkDomain (1.18e9-59.5*156250, 56*156250, 0., 1e12);
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  pre1.initSolvableParms (solveDomains);

  // Get the ParmData from the Prediffer and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (1, solveDomains, pre1.getWorkDomain());
  const ParmDataInfo& pData = pre1.getSolvableParmData();
  solver.setSolvableParmData (pData, 0);
  pre1.showSettings();
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(15)
       << solver.getSolvableValues(0) << endl;

  for (int it=0; it<maxniter; ++it) {
    // Get the fitter from the prediffer and give them to the solver.
    vector<casa::LSQFit> fitters;
    pre1.fillFitters (fitters, "DATA");
    solver.mergeFitters (fitters, 0);
    // Do the solve.
    solver.solve (true);
    cout << "iter" << it << ":  " << setprecision(15)
	 << solver.getSolvableValues(0) << endl;
    const Quality& quality = solver.getQuality(0);
    cout << quality << endl;
    cout.precision (prec);
    pre1.updateSolvableParms (solver.getSolvableParmData());
    // Stop if converged.
    if ((abs(quality.itsFit) <= epsilon) && quality.itsFit <= 0.0) {
      break;
    }
  }
  pre1.writeParms();
}

// Solve for steps in time.
void doSolveStep (Prediffer& pre1, const vector<string>& solv,
		  const vector<string>& solvexc, double timeStep,
		  int maxniter, double epsilon=1e-4)
{
  // Set the solvable parameters.
  pre1.clearSolvableParms();
  pre1.setSolvableParms (solv, solvexc);
  // Set start time (take into account that first time stamp is 20).
  double timeStart = 4.47203e+09-4260-15-5;
  double timeLast = timeStart + 43075+25;
    ///double timeLast = timeStart + 120;
    ///timeStart -=  30-20;
  // Loop through all time domains.
  int counter=0;
  double st=timeStart;
  pre1.lock();
  while (timeStart < timeLast) {
    cout << "timecounter=" << counter++ << ' ' << timeStart-st << endl;
    // Set a domain. Use middle 56 channels and a few times per step.
    ///pre1.setWorkDomain (1170078125+24*156250, 16*156250, timeStart, timeStep);
    pre1.setWorkDomain (4, 59,timeStart, timeStep);
    ///pre1.setWorkDomain (1.18e9-59.5*156250, 56*156250, timeStart, timeStep);
    vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
    pre1.initSolvableParms (solveDomains);

    // Get the ParmData from the Prediffer and send it to the solver.
    Solver solver;
    solver.initSolvableParmData (1, solveDomains, pre1.getWorkDomain());
    solver.setSolvableParmData (pre1.getSolvableParmData(), 0);
    pre1.showSettings();
    streamsize prec = cout.precision();
    cout << "Before: " << setprecision(15)
	 << solver.getSolvableValues(0) << endl;

    for (int it=0; it<maxniter; ++it) {
      // Get the fitter from the prediffer and give them to the solver.
      vector<casa::LSQFit> fitters;
      pre1.fillFitters (fitters, "DATA");
      solver.mergeFitters (fitters, 0);
      // Do the solve.
      solver.solve (true);
      cout << "iter" << it << ":  " << setprecision(15)
	   << solver.getSolvableValues(0) << endl;
      const Quality& quality = solver.getQuality(0);;
      cout << quality << endl;
      cout.precision (prec);
      pre1.updateSolvableParms (solver.getSolvableParmData());
      // Stop if converged.
      if ((abs(quality.itsFit) <= epsilon) && quality.itsFit <= 0.0) {
	break;
      }
    }
    pre1.writeParms();
    timeStart += timeStep;
  }
  pre1.unlock();
}

// Subtract the sources.
void doSubtract (Prediffer& pre1, double timeStep)
{
  // Set start time (take 10 sec into account for first time stamp of 20).
  double timeStart = 4.47203e+09-4260-10;
  double timeLast = timeStart + 43075+25;
  ///  timeLast = timeStart + 120;
  timeStart -=  30-20;
  // Loop through all time domains.
  while (timeStart < timeLast) {
    // Set a domain. Use middle 56 channels and 20 times per step.
    pre1.setWorkDomain (4, 59, timeStart, timeStep);
    ///pre1.setWorkDomain (1.18e9-59.5*156250, 56*156250, timeStart, timeStep);
    pre1.showSettings();
    // Subtract the model.
    pre1.subtractData ("DATA", "CORRECTED_DATA");
    timeStart += timeStep;
  }
}

// Subtract the sources.
void doCorrect (Prediffer& pre1, double timeStep)
{
  // Set start time (take 10 sec into account for first time stamp of 20).
  double timeStart = 4.47203e+09-4260-10;
  double timeLast = timeStart + 43075+25;
  ///  timeLast = timeStart + 120;
  timeStart -=  30-20;
  // Loop through all time domains.
  while (timeStart < timeLast) {
    // Set a domain. Use middle 56 channels and 20 times per step.
    pre1.setWorkDomain (4, 59, timeStart, timeStep);
    ///pre1.setWorkDomain (1.18e9-59.5*156250, 56*156250, timeStart, timeStep);
    pre1.showSettings();
    // Subtract the model.
    pre1.correctData ("CORRECTED_DATA", "CORRECTED_DATA");
    timeStart += timeStep;
  }
}


int main (int argc, const char* argv[])
{
  cout << ">>>" << endl;
  INIT_LOGGER("t3C343");
  BBSTest::Logger::init();
  try {
    if (argc < 6) {
      cerr << "Run as: t3C343 user msname meqparmtable skyparmtable model [nrgrp=1] [type=1] [nriter=1] [calcuvw=1] [dbtype=aips]"
	   << endl;
      return 1;
    }
    int nrgrp=1;
    if (argc > 6) {
      istringstream iss(argv[6]);
      iss >> nrgrp;
    }
    int type=1;
    if (argc > 7) {
      istringstream iss(argv[7]);
      iss >> type;
    }
    int maxniter=1;
    if (argc > 8) {
      istringstream iss(argv[8]);
      iss >> maxniter;
    }
    int calcuvw=1;
    if (argc > 9) {
      istringstream iss(argv[9]);
      iss >> calcuvw;
    }
    string dbtype="aips";
    if (argc > 10) {
      dbtype = argv[10];
    }

    cout << "t3C343 user=         " << argv[1] << endl;
    cout << "       msname:       " << argv[2] << endl;
    cout << "       meqparmtable: " << argv[3] << endl;
    cout << "       skyparmtable: " << argv[4] << endl;
    cout << "       modeltype:    " << argv[5] << endl;
    cout << "       nrsrcgrp:     " << nrgrp << endl;
    cout << "       solve-type:   " << type << endl;
    cout << "       maxniter:     " << maxniter << endl;
    cout << "       calcuvw:      " << calcuvw << endl;
    cout << "       dbtype:       " << dbtype << endl;

    // Read the info for the ParmTables
    ParmDB::ParmDBMeta meqPdm(dbtype, argv[3]);
    ParmDB::ParmDBMeta skyPdm(dbtype, argv[4]);

    // Do a solve.
    {
      ///      vector<int> antVec(5);
      ///antVec[0]=0; antVec[1]=3; antVec[2]=6; antVec[3]=9; antVec[4]=13;
      ///vector<int> antVec(2);
      ///antVec[0]=4; antVec[1]=8;
      vector<int> antVec(14);
      for (int i=0; i<14; i++) {
      antVec[i] = i;
      }
      vector<vector<int> > srcgrp;
      if (nrgrp == 1) {
	vector<int> grp1;
	grp1.push_back (1);
	grp1.push_back (2);
	srcgrp.push_back (grp1);
      }
      Prediffer pre1(argv[2], meqPdm, skyPdm, 
		     antVec, argv[5], srcgrp, calcuvw);
      // Do a further selection; only XX,YY and no autocorrelations.
      vector<int> corr(2,0);
      corr[1] = 3;
      vector<int> antVec2;
      pre1.select (antVec2, antVec2, false, corr);
      if (type == 1) {
	vector<string> solv(2);
	solv[0] = "StokesI.*";
	solv[1] = "StokesQ.*";
	//solv[1] = "StokesQ.CP2";
	//solv[0] = "StokesI.*";
	//      solv[1] = "RA.*";
	//      solv[2] = "DEC.*";
	///doSolveStep (pre1, solv, vector<string>(), 60, maxniter);
	doSolveAll (pre1, solv, maxniter);
      } else if (type == 2) {
	// Keep phases for station 1 fixed at 0.
	vector<string> solv2(2);
	vector<string> solv2exc(4);
	solv2[0] = "EJ11.phase.*";
	solv2[1] = "EJ22.phase.*";
	solv2exc[0] = "EJ11.phase.SR1.*";
	solv2exc[1] = "EJ11.phase.SR1";
	solv2exc[2] = "EJ22.phase.SR1.*";
	solv2exc[3] = "EJ22.phase.SR1";
	doSolveStep (pre1, solv2, solv2exc, 30, maxniter);
      } else if (type == 3) {
	vector<string> solv2(2);
	solv2[0] = "EJ11.ampl.*";
	solv2[1] = "EJ22.ampl.*";
	///doSolveStep (pre1, solv2, vector<string>(), 60, maxniter);
	doSolveStep (pre1, solv2, vector<string>(), 900, maxniter);
      } else if (type == 4) {
	// Let SVD sort out the phase ambiguity.
	vector<string> solv2(2);
	solv2[0] = "EJ11.phase.*";
	solv2[1] = "EJ22.phase.*";
	doSolveStep (pre1, solv2, vector<string>(), 30, maxniter);
      } else if (type == 5) {
	cout << "correct data ..." << endl;
        doCorrect (pre1, 900);
      } else {
	cout << "subtracct data ..." << endl;
        doSubtract (pre1, 900);
      }
    }
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "<<<" << endl;
  return 0;
}
