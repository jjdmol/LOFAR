//# tMWControl.cc: Test program for Master-Worker framework
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
#include "PredifferTest.h"
#include "SolverTest.h"
#include <MWControl/MWParameterHandler.h>
#include <MWControl/MWStrategySpec.h>
#include <MWCommon/MPIConnection.h>
#include <MWCommon/Controller.h>
#include <MWCommon/MWMultiStep.h>
#include <MWControl/MWLocalSpec.h>
#include <MWControl/MWGlobalSpec.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;


class Runner
{
public:
  explicit Runner (const string& parsetName)
    : itsParams (ParameterSet(parsetName))
  {
    // Define the functions creating the proxy workers.
    // The names localWorker and globalWorker are mandatory.
    itsFactory.push_back ("LocalWorker", PredifferTest::create);
    itsFactory.push_back ("GlobalWorker", SolverTest::create);
  }

  string getDataSetName() const
    { return itsParams.getDataSetName(); }

  const WorkerFactory& getFactory() const
    { return itsFactory; }

  ParameterSet getParSet() const
    { return itsParams; }

  void run (MasterControl& mc)
  {
    // Assemble all steps defined in the parameters into a single spec.
    vector<MWStrategySpec> strategySpecs = itsParams.getStrategies();
    // Loop through all strategies.
    for (vector<MWStrategySpec>::const_iterator iter=strategySpecs.begin();
	 iter!=strategySpecs.end();
	 ++iter) {
      mc.setWorkDomainSpec (iter->getWorkDomainSpec());
      // Execute the steps.
      mc.processSteps (iter->getSteps());
    }
  }

private:
  MWParameterHandler itsParams;
  WorkerFactory      itsFactory;
};



// Get the socket info from the arguments.
void findSocket (int argc, const char** argv,
                 string& host, string& port, int& nnode, int& rank)
{
  ASSERTSTR (argc >= 6, "Using sockets run as: tMWControl socket <host> "
	     "<port> <#processes> <rank>");
  host = argv[2];
  port = argv[3];
  istringstream iss(argv[4]);
  iss >> nnode;
  istringstream iss1(argv[5]);
  iss1 >> rank;
}

int main (int argc, const char** argv)
{
  // Register the create functions for the various steps.
  MWLocalSpec::registerCreate();
  MWGlobalSpec::registerCreate();
  MWMultiStep::registerCreate();
  // Define the functions to use for the proxy workers.
  WorkerFactory factory;
  factory.push_back ("LocalWorker", PredifferTest::create);
  factory.push_back ("GlobalWorker", SolverTest::create);
  // Initialize MPI (also succeeds if no MPI available).
  MPIConnection::initMPI (argc, argv);
  // Create the object to run all the steps.
  Runner runner("tMWControl.in");
  // Create and run the controller.
  Controller<Runner> ctrl(runner, "tMWControl_tmp.cdesc",
			  "tMWControl_tmp.cout");
  int status = 0;
  if (argc > 1  &&  string(argv[1]) == "socket") {
    string host, port;
    int nnode, rank;
    findSocket (argc, argv, host, port, nnode, rank);
    status = ctrl.execute (host, port, nnode, rank);
  } else {
    status = ctrl.execute ();
  }
  MPIConnection::endMPI();
  exit(status);
}
