//# BBSrun.cc: Main program to do a BBS run
//#
//# Copyright (C) 2006
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
#include <ParmDB/ParmDB.h>
#include <MS/MSDesc.h>
#include <APS/ParameterSet.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobIBufStream.h>
#include <Common/LofarLogger.h>
#include <Common/VectorUtil.h>

#include <casa/Exceptions/Error.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::ParmDB;
using namespace LOFAR::ACC::APS;


void predict (Prediffer& prediffer, const MSDesc& msd,
	      const StepProp& stepProp,
	      double timeStep, int startChan, int endChan)
{
  double time = msd.startTime;
  double endTime = msd.endTime;
  while (time < endTime) {
    prediffer.setWorkDomain (startChan, endChan, time, timeStep);
    prediffer.setStepProp (stepProp);
    prediffer.writePredictedData();
    time += timeStep;
  }
}           

void subtract (Prediffer& prediffer, const MSDesc& msd,
	       const StepProp& stepProp,
	       double timeStep, int startChan, int endChan)
{
  double time = msd.startTime;
  double endTime = msd.endTime;
  while (time < endTime) {
    prediffer.setWorkDomain (startChan, endChan, time, timeStep);
    prediffer.setStepProp (stepProp);
    prediffer.subtractData();
    time += timeStep;
  }
}           

void solve (Prediffer& prediffer, const MSDesc& msd,
	    const StepProp& stepProp,
	    double timeStep, int startChan, int endChan,
	    const SolveProp& solveProp,
	    const vector<int32>& nrinterval,
	    bool saveSolution)
{
  double time = msd.startTime;
  double endTime = msd.endTime;
  SolveProp solProp(solveProp);
  while (time < endTime) {
    // Use given channels and time steps.
    prediffer.setWorkDomain (startChan, endChan, time, timeStep);
    prediffer.setStepProp (stepProp);
    // Form the solve domains.
    const MeqDomain& workDomain = prediffer.getWorkDomain();
    vector<MeqDomain> solveDomains;
    double freq = workDomain.startX();
    double stepf = (workDomain.endX() - freq) / nrinterval[0];
    double stept = (workDomain.endY() - time) / nrinterval[1];
    double sdTime = workDomain.startY();
    for (int i=0; i<nrinterval[1]; ++i) {
      double sdFreq = workDomain.startX();
      for (int j=0; j<nrinterval[0]; ++j) {
	solveDomains.push_back (MeqDomain(sdFreq, sdFreq+stepf,
					  sdTime, sdTime+stept));
	sdFreq += stepf;
      }
      sdTime += stept;
    }
    solProp.setDomains (solveDomains);
    prediffer.setSolveProp (solProp);

    Solver solver;
    solver.initSolvableParmData (1, solveDomains, prediffer.getWorkDomain());
    solver.setSolvableParmData (prediffer.getSolvableParmData(), 0);
    prediffer.showSettings();
    cout << "Before: " << setprecision(10)
	 << solver.getSolvableValues(0) << endl;
    
    for(int i=0; i<solProp.getMaxIter(); ++i) {
      // Get the fitter data from the prediffer and give it to the solver.
      vector<casa::LSQFit> fitters;
      prediffer.fillFitters (fitters);
      solver.mergeFitters (fitters, 0);

      // Do the solve.
      solver.solve(false);
      cout << "iteration " << i << ":  " << setprecision(10)
	   << solver.getSolvableValues(0) << endl;
      cout << solver.getQuality(0) << endl;

      prediffer.updateSolvableParms (solver.getSolvableParmData());
    }
    if (saveSolution) {
      cout << "Writing solutions into ParmDB ..." << endl;
      prediffer.writeParms();
    }
    time += timeStep;
  }
}


bool doIt (const string& parsetName)
{
  string user, measurementSet, skyPDB, instrumentPDB, instrumentModel;
  string operation, columnNameIn, columnNameOut;
  double timeDomainSize;
  vector<string> solvParms, exclParms;
  vector<int32> nrSolveInterval;
  vector<int32> antennas;
  vector<bool> corrs;
  int startChan, endChan, nriter;
  bool calcUVW, saveSolution;
  
  // Read & parse parameters
  try
  {
    ParameterSet parameters(parsetName);
    
    user = parameters.getString ("user");
    instrumentPDB = parameters.getString ("instrument_parmdb");
    skyPDB = parameters.getString ("sky_parmdb");
    measurementSet = parameters.getString ("measurement_set");
    instrumentModel = parameters.getString ("instrument_model");
    calcUVW = parameters.getBool ("calculate_UVW");
    operation = parameters.getString ("operation");
    columnNameIn = parameters.getString ("data_column_in");
    columnNameOut = parameters.getString ("data_column_out");
    timeDomainSize = parameters.getDouble ("time_domain_size");
    startChan = parameters.getInt32 ("start_channel");
    endChan = parameters.getInt32 ("end_channel");
    antennas = parameters.getInt32Vector ("antennas");
    corrs = parameters.getBoolVector ("corrs");
    solvParms = parameters.getStringVector ("solvable_parms");
    exclParms = parameters.getStringVector ("solvable_parms_excluded");
    nrSolveInterval = parameters.getInt32Vector ("nr_solve_interval");
    nriter = parameters.getInt32 ("nriter");
    saveSolution = parameters.getBool ("save_solution");
  }
  catch (exception& _ex)
  {
    cout << "Parameter read or parse error: " << _ex.what() << endl;
    return false;
  }
  
  cout << "user                   : " << user << endl;
  cout << "instrument ParmDB      : " << instrumentPDB << endl;
  cout << "sky ParmDB             : " << skyPDB << endl;
  cout << "measurement set        : " << measurementSet << endl;
  cout << "instrument model       : " << instrumentModel << endl;
  cout << "calculate UVW          : " << calcUVW << endl;
  cout << "start channel          : " << startChan << endl;
  cout << "end channel            : " << endChan << endl;
  cout << "operation              : " << operation << endl;
  cout << "time domain size       : " << timeDomainSize << endl;
  
  // Get meta data from description file.
  string name(measurementSet+"/vis.des");
  std::ifstream istr(name.c_str());
  ASSERTSTR (istr, "File " << measurementSet
	     << "/vis.des could not be opened");
  BlobIBufStream bbs(istr);
  BlobIStream bis(bbs);
  MSDesc msd;
  bis >> msd;

  // Construct prediffer.
  Prediffer prediffer(measurementSet, 
		      ParmDBMeta("aips", instrumentPDB),
		      ParmDBMeta("aips", skyPDB),
		      0,
		      calcUVW);
  // Set strategy.
  StrategyProp stratProp;
  stratProp.setAntennas (antennas);
  stratProp.setCorr (corrs);
  stratProp.setInColumn (columnNameIn);
  ASSERT (prediffer.setStrategyProp (stratProp));
  // Fill step properties.
  StepProp stepProp;
  stepProp.setModel (StringUtil::split(instrumentModel,'.'));
  stepProp.setOutColumn (columnNameOut);
  try {
    if (operation == "solve") {
      ASSERT (nrSolveInterval.size()==2);
      ASSERT (nrSolveInterval[0] > 0  &&  nrSolveInterval[1] > 0);
      cout << "input column name      : " << columnNameIn << endl;
      cout << "solvable parms         : " << solvParms << endl;
      cout << "solvable parms excluded: " << exclParms << endl;
      cout << "solve nrintervals      : " << nrSolveInterval << endl;
      cout << "solve nriter           : " << nriter << endl;
      SolveProp solveProp;
      solveProp.setParmPatterns (solvParms);
      solveProp.setExclPatterns (exclParms);
      solveProp.setMaxIter (nriter);
      solve (prediffer, msd, stepProp,
	     timeDomainSize, startChan, endChan,
	     solveProp,
	     nrSolveInterval, saveSolution);
    } else if (operation == "predict") {
      cout << "output column name     : " << columnNameOut << endl;
      predict (prediffer, msd, stepProp,
	       timeDomainSize, startChan, endChan);
    } else if (operation == "subtract") {
      cout << "input column name      : " << columnNameIn << endl;
      cout << "output column name     : " << columnNameOut << endl;
      subtract (prediffer, msd, stepProp,
		timeDomainSize, startChan, endChan);
    } else {
      cout << "Only operations solve, predict, and subtract are valid" << endl;
      return 1;
    }
  }
  catch (exception& _ex)
  {
    cout << "error: " << _ex.what() << endl;
    return false;
  }
  return true;
}

int main (int argc, const char* argv[])
{
  if (argc < 2) {
    cout << "Run as: BBSrun parset-name" << endl;
    return 1;
  }
  if (doIt (argv[1])) {
    cout << "OK" << endl;
    return 0; 
  }
  return 1;
}
