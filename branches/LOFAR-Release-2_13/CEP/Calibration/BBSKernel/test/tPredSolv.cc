//# tPredSolv.cc: Test program for Prediffer and Solver classes
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
#include <BBSKernel/Prediffer.h>
#include <BBSKernel/Solver.h>
#include <BBSKernel/ParmWriter.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <ParmDB/ParmDB.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <malloc.h>

#include <Common/hexdump.h>


using namespace LOFAR;
using namespace LOFAR::BBS;
using namespace std;

// Note:
// demo3.MS contains 50 frequency channels of 500000 Hz with
// center frequencies of 137750000-162250000 Hz.
// There are 5 time stamps of 2 sec in it (centers 2.35208883e9 + 2-10).

void doSolve (Prediffer& pre1,
	      const StepProp& stepProp, const SolveProp& solveProp)
{
  // Set a domain.
  cout << ">>>" << endl;
  ASSERT (pre1.setWorkDomain (0, 1, 0., 1e12));
  ASSERT (pre1.setStepProp (stepProp));
  // Have a single solve domain.
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  // Set the solvable parameters and solve domains.
  SolveProp sprop(solveProp);
  sprop.setDomains (solveDomains);
  ASSERT (pre1.setSolveProp (sprop));
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

  for (int it=0; it<sprop.getMaxIter(); ++it) {
    // Get the fitter from the prediffer and give it to the solver.
    vector<casa::LSQFit> fitters;
    cout << ">>>" << endl;
    pre1.fillFitters (fitters);
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
	       const StepProp& stepProp, const SolveProp& solveProp)
{
  // Set a domain.
  cout << ">>>" << endl;
  ASSERT (pre1.setWorkDomain (0, 3, 0., 1e12));
  ASSERT (pre2.setWorkDomain (0, 3, 0., 1e12)); 
  ASSERT (pre1.setStepProp (stepProp));
  ASSERT (pre2.setStepProp (stepProp));
  // Have a single solve domain.
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  // Set the solvable parameters and solve domains.
  SolveProp sprop(solveProp);
  sprop.setDomains (solveDomains);
  ASSERT (pre1.setSolveProp (sprop));
  cout << "<<<" << endl;
  // Get the ParmData from the Prediffers and send it to the solver.
  Solver solver;
  solver.initSolvableParmData (2, solveDomains, pre1.getWorkDomain());
  solver.setSolvableParmData (pre1.getSolvableParmData(), 0);
  solver.setSolvableParmData (pre2.getSolvableParmData(), 1);
    
  streamsize prec = cout.precision();
  cout << "Before: " << setprecision(10) << solver.getSolvableValues(0)
       << endl;
  for (int it=0; it<sprop.getMaxIter(); ++it) {
    // Get the fitter from the prediffer and give it to the solver.
    vector<casa::LSQFit> fitters;
    cout << ">>>" << endl;
    pre1.fillFitters (fitters);
    solver.mergeFitters (fitters, 0);
    pre2.fillFitters (fitters);
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

void doSolve1 (Prediffer& pre1,
	       const StepProp& stepProp, const SolveProp& solveProp)
{
  // Set a domain.
  cout << ">>>" << endl;
  ASSERT (pre1.setWorkDomain (0, 3, 0., 1e12));
  ASSERT (pre1.setStepProp (stepProp));
  // Have a single solve domain.
  vector<MeqDomain> solveDomains(1, pre1.getWorkDomain());
  // Set the solvable parameters and solve domains.
  SolveProp sprop(solveProp);
  sprop.setDomains (solveDomains);
  ASSERT (pre1.setSolveProp (sprop));
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

  for (int it=0; it<sprop.getMaxIter(); ++it) {
    // Get the fitter from the prediffer and give it to the solver.
    vector<casa::LSQFit> fitters;
    cout << ">>>" << endl;
    pre1.fillFitters (fitters);
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
      Prediffer pre1(argv[2], meqPdm, skyPdm, 0, false);
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      StrategyProp stratProp;
      stratProp.setAntennas (antVec);
      stratProp.setInColumn ("DATA");
      ASSERT (pre1.setStrategyProp (stratProp));
      // Do a further selection of a few stations.
      vector<vector<int> > baselines(1);
      vector<int>& antVec2 = baselines[0];
      antVec2.resize(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      StepProp stepProp;
      stepProp.setModel (StringUtil::split("TOTALGAIN.REALIMAG",'.'));
      stepProp.setBaselines (baselines, baselines);
      vector<string> solv(3);
      solv[0] = "RA:*";
      solv[1] = "DEC:*";
      solv[2] = "StokesI:*";
      SolveProp solveProp;
      solveProp.setParmPatterns (solv);
      solveProp.setMaxIter (9);
      doSolve (pre1, stepProp, solveProp);
      cout << "End of first test" << endl;
    }
    // Do a solve using 2 prediffers.
    {
      cout << "Starting test with two prediffers" << endl;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 0, false);
      Prediffer pre2(argv[2], meqPdm, skyPdm, 0, false);
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      StrategyProp stratProp;
      stratProp.setAntennas (antVec);
      stratProp.setInColumn ("DATA");
      ASSERT (pre1.setStrategyProp (stratProp));
      ASSERT (pre2.setStrategyProp (stratProp));
      // Do a further selection of a few stations.
      vector<vector<int> > baselines(1);
      vector<int>& antVec2 = baselines[0];
      antVec2.resize(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      StepProp stepProp;
      stepProp.setModel (StringUtil::split("TOTALGAIN.REALIMAG",'.'));
      stepProp.setBaselines (baselines, baselines);
      vector<string> solv(3);
      solv[0] = "RA:*";
      solv[1] = "DEC:*";
      solv[2] = "StokesI:*";
      SolveProp solveProp;
      solveProp.setParmPatterns (solv);
      solveProp.setMaxIter (1);
      doSolve2 (pre1, pre2, stepProp, solveProp);
      cout << "End of test with two prediffers" << endl;
    }
    // Take more baselines.
    {
      cout << "Starting test with 21 antennas" << endl;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 0, false);
      vector<int> antVec(21);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 4*i;
      }
      StrategyProp stratProp;
      stratProp.setAntennas (antVec);
      stratProp.setInColumn ("DATA");
      ASSERT (pre1.setStrategyProp (stratProp));
      // Only use first correlation.
      vector<bool> corrVec(4, false);
      corrVec[0] = true;
      StepProp stepProp;
      stepProp.setModel (StringUtil::split("TOTALGAIN.REALIMAG",'.'));
      stepProp.setCorr (corrVec);
      vector<string> solv(3);
      solv[0] = "RA:*";
      solv[1] = "DEC:*";
      solv[2] = "StokesI:*";
      SolveProp solveProp;
      solveProp.setParmPatterns (solv);
      solveProp.setMaxIter (5);
      doSolve (pre1, stepProp, solveProp);
      cout << "End of test with 21 antennas" << endl;
    }
    // Do a solve updating the parm table.
    // This should be the last one.
    {
      cout << "Starting test with updating parmtable" << endl;
      Prediffer pre1(argv[2], meqPdm, skyPdm, 0, false);
      vector<int> antVec(10);
      for (uint i=0; i<antVec.size(); ++i) {
	antVec[i] = 2*i;
      }
      StrategyProp stratProp;
      stratProp.setAntennas (antVec);
      stratProp.setInColumn ("DATA");
      ASSERT (pre1.setStrategyProp (stratProp));
      // Do a further selection of a few stations.
      vector<vector<int> > baselines(1);
      vector<int>& antVec2 = baselines[0];
      antVec2.resize(10);
      for (uint i=0; i<antVec2.size(); ++i) {
	antVec2[i] = 4*i;
      }
      StepProp stepProp;
      stepProp.setModel (StringUtil::split("TOTALGAIN.REALIMAG",'.'));
      stepProp.setBaselines (baselines, baselines);
      vector<string> solv(3);
      solv[0] = "RA:*";
      solv[1] = "DEC:*";
      solv[2] = "StokesI:*";
      SolveProp solveProp;
      solveProp.setParmPatterns (solv);
      solveProp.setMaxIter (5);
      doSolve1 (pre1, stepProp, solveProp);
      cout << "End of test with updating parmtable" << endl;
    }

  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
