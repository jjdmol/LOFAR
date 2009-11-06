//# ControllerBase.h: Base class to execute the master and the workers
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
//#  $Id$

#ifndef LOFAR_MWCOMMON_CONTROLLERBASE_H
#define LOFAR_MWCOMMON_CONTROLLERBASE_H

// @file Base class to execute the master and the workers
// @brief 
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <MWCommon/MWConnectionSet.h>
#include <MWCommon/WorkerFactory.h>
#include <MWCommon/MasterControl.h>
#include <MWCommon/VdsDesc.h>
#include <MWCommon/MWIos.h>

namespace LOFAR { namespace CEP {

  //# Forward Declarations.
  class VdsDesc;

  // @ingroup MWCommon
  // @brief Base class to execute the master and the workers

  // This non-templated class factors out all non-templated code of
  // the Controller class.

  class ControllerBase
  {
  public:
    // Construct the controller to process the given (distributed) data set
    // on the given cluster (using its cluster description name).
    // Standard output is logged in the given log file.
    ControllerBase (const WorkerFactory&,
		    const ParameterSet&,
		    const string& dsDescName,
		    const string& clusterDescName,
		    const string& logFileName);
    
    virtual ~ControllerBase();

    // Execute the run using sockets or a single process.
    // If the host or port name is empty, a single process is used.
    // If sockets are used, it is assumed that <tt>nproc</tt> identical
    // processes (including master) have been started. Each process should
    // have a unique rank, where rank 0 will be the master.
    // <br>It returns a non-zero value on failure.
    int execute (const string& host, const string& port,
		 int nproc, int rank);

    // Execute the run using MPI or a single process.
    // <br>It returns a non-zero value on failure.
    int execute();

  private:
    // Let the Runner in the derived class run.
    virtual void run (MasterControl&) = 0;

    // Do the execute using MPI, sockets, or single process.
    int doExecute (const string& host, const string& port,
		   int nrNode, int rank, bool useMPI);

    // Run the master process.
    void runMaster (const string& port,
		    int globalWorkerRank,
		    int nworkers, int nparts);

    // Run a local worker.
    void runLocalWorker (const string& host, const string& port);

    // Run a global worker.
    void runGlobalWorker (const string& host, const string& port);

    // Setup all the workers.
    // Find out what they can do, i.e. if they work locally or globally.
    // It returns a vector telling which data part is handled by local worker i.
    std::vector<std::string> setAllWorkers (MWConnectionSet& workers,
					    int nworkers);

    //# Data members
    WorkerFactory          itsFactory;
    ParameterSet           itsParSet;
    VdsDesc                itsDsDesc;
    std::string            itsClusterName;
    std::string            itsLogName;
    MWConnectionSet::ShPtr itsLocalWorkers;
    MWConnectionSet::ShPtr itsGlobalWorkers;
  };

}} //# end namespaces

#endif
