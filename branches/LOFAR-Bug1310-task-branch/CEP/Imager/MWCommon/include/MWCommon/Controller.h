//# Controller.h: Class to execute the master and the workers
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
//#  $Id$

#ifndef LOFAR_MWCOMMON_CONTROLLER_H
#define LOFAR_MWCOMMON_CONTROLLER_H

// @file
// @brief Class to execute the master and the workers
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <MWCommon/ControllerBase.h>

namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Class to execute the master and the workers

  // This templated class does the execution of the master and the local and
  // global workers. A local worker is the worker that operates on the data.
  // A global worker is used to combine results from local workers (e.g.
  // to do a global solve).
  // The class is used by the master as well as the workers. The process rank
  // determines if it is used as the master (rank 0) or a worker.
  // The <tt>execute</tt> functions does most of the work. A so-called
  // Runner object (the template parameter) is used to do application
  // specific operations.
  // The <tt>execute</tt> function operates in several stages:
  // <ol>
  //  <li> It uses the Runner object to ask for the data set to process.
  //    Such a data set is a description file (handled by VdsDesc) telling
  //    the number of data set parts and where each part resides.
  //  <li> The connections between master and workers are set up. It will use
  //    MPI if compiled in and required. Otherwise it can use sockets.
  //    If there is only one process, everything will run in that single
  //    process and memory connections will be used. This is particularly
  //    useful for debugging.
  //    It is checked if there is a local worker for each data set part.
  //  <li> The processes will start executing. The workers wait for messages
  //    and act upon it. A message contains a type (see MasterControl) and
  //    usually an MWStep object. That object tells the worker what to do.
  //    A factory (WorkerFactory) is used to create the correct MWStep object.
  //    In this way the framework is very general and any step can be used.
  //    <br>The master will do the following:
  //    <ol>
  //     <li> Obtain the domain of the data set and send it with the
  //       ParameterSet obtained from the Runner to the workers to let
  //       them initialise themselves.
  //     <li> Ask all workers what kind of work they can perform.
  //     <li> Ask the Runner to do the application-specific work. This is
  //       done by calling the appropriate functions in MasterControl.
  //       They will send commands to the workers and act upon their replies.
  //     <li> After the Runner has finished, it quits MasterControl which
  //       sends quit messages to the workers.
  //    </ol>
  // </ol>

  // <example>
  // Here follows an example of a Runner class taken from the test
  // program tMWControl. It shows the functions which have to be
  // defined in the class. Also the copy constructor must be available
  // (in this example it is implemented by the compiler).
  // <srcblock>
  // class Runner
  // {
  // public:
  //   explicit Runner (const string& parsetName)
  //     : itsParams (ParameterSet(parsetName))
  //   {
  //     // Define the functions creating the proxy workers.
  //     // The names localWorker and globalWorker are mandatory.
  //     itsFactory.push_back ("LocalWorker", PredifferTest::create);
  //     itsFactory.push_back ("GlobalWorker", SolverTest::create);
  //   }
  // 
  //   string getDataSetName() const
  //     { return itsParams.getDataSetName(); }
  // 
  //   const WorkerFactory& getFactory() const
  //     { return itsFactory; }
  // 
  //   ParameterSet getParSet() const
  //     { return itsParams; }
  // 
  //   void run (MasterControl& mc)
  //   {
  //     // Assemble all steps defined in the parameters into a single spec.
  //     vector<MWStrategySpec> strategySpecs = itsParams.getStrategies();
  //     // Loop through all strategies.
  //     for (vector<MWStrategySpec>::const_iterator iter=strategySpecs.begin();
  // 	 iter!=strategySpecs.end();
  // 	 ++iter) {
  //       mc.setWorkDomainSpec (iter->getWorkDomainSpec());
  //       // Execute the steps.
  //       mc.processSteps (iter->getSteps());
  //     }
  //   }
  //
  // private:
  //   MWParameterHandler itsParams;
  //   WorkerFactory      itsFactory;
  // };
  // </srcblock>
  // </example>

  template<typename Runner>
  class Controller : public ControllerBase
  {
  public:
    // Construct the controller to process the given (distributed) data set
    // on the given cluster (using its cluster description name).
    // Standard output is logged in the given log file.
    Controller (const Runner& runner,
		const string& clusterName,
		const string& logFileName)
      : ControllerBase (runner.getFactory(),
			runner.getParSet(),
			runner.getDataSetName(),
			clusterName,
			logFileName),
	itsRunner (runner)
    {}

    virtual ~Controller()
    {}

    // Let the Runner run.
    virtual void run (MasterControl& mc)
      { itsRunner.run (mc); }

  private:
    //# Data members
    Runner itsRunner;
  };

}} //# end namespaces

#endif
