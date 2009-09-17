//#  ControllerBase.cc: Base class to execute the master and workers
//#
//#  Copyright (C) 2008
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

#include <lofar_config.h>
#include <MWCommon/ControllerBase.h>
#include <MWCommon/MemConnectionSet.h>
#include <MWCommon/SocketConnectionSet.h>
#include <MWCommon/MPIConnectionSet.h>
#include <MWCommon/WorkerControl.h>
#include <MWCommon/WorkersDesc.h>
#include <MWCommon/ClusterDesc.h>
#include <MWCommon/MWError.h>
#include <MWCommon/MWIos.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

    ControllerBase::ControllerBase (const WorkerFactory& factory,
				    const ParameterSet& parSet,
				    const string& dsDescName,
				    const string& clusterName,
				    const string& logFileName)
      : itsFactory     (factory),
	itsParSet      (parSet),
	itsDsDesc      (dsDescName),
	itsClusterName (clusterName),
	itsLogName     (logFileName)
    {}

    ControllerBase::~ControllerBase()
    {}

    int ControllerBase::execute()
    {
      return doExecute (string(), string(), 0, 0, true);
    }

    int ControllerBase::execute (const string& host, const string& port,
				 int nproc, int procRank)
    {
      return doExecute (host, port, nproc, procRank, false);
    }

    int ControllerBase::doExecute (const string& host, const string& port,
				   int nproc, int procRank, bool useMPI)
    {
      int status = 0;
      try {
	// Find nr of nodes (processes) and rank.
	int nnode = 0;
	int rank  = 0;
	if (useMPI) {
	  nnode = MPIConnection::getNrNodes();
	  rank  = MPIConnection::getRank();
	}
	// If only one MPI node, we may run in a single process
	// or in multiple processes connected via sockets.
	// Find out from arguments.
	if (nnode <= 1  &&  !port.empty()) {
	  nnode = nproc;
	  rank  = procRank;
	}
	ASSERT (rank < nnode);
	{
	  // Set the name of the output stream.
	  std::ostringstream ostr;
	  ostr << rank;
	  MWIos::setName (itsLogName + ostr.str());
	}
	// Open the parameter set and get nr of VDS parts.
	int nparts = itsDsDesc.getParts().size();
	// Find out if this process is master, globalWorker, or localWorker.
	int globalWorkerRank = 0;
	if (nnode > nparts+1) {
	  // More workers than data parts, so the master and globalWorker
	  // can run on different nodes.
	  globalWorkerRank = 1;
	}
	// Initialize and run the controls.
	if (rank == 0) {
	  runMaster (port, globalWorkerRank, nnode-1, nparts);
	} else if (rank > globalWorkerRank) {
	  runLocalWorker (host, port);
	} else {
	  runGlobalWorker (host, port);
	}
      } catch (std::exception& x) {
	cerr << "Unexpected exception in Controller::doExecute: "
	     << x.what() << endl;
	status = 1;
      }
      return status;
    }

    void ControllerBase::runMaster (const string& port,
				    int globalWorkerRank,
				    int nworkers, int nparts)
    {
      // Get the full observation domain for this data set.
      const VdsPartDesc& vdsDesc = itsDsDesc.getDesc();
      ObsDomain fullDomain;
      fullDomain.setTime (vdsDesc.getStartTime(),
			  vdsDesc.getEndTime());
      fullDomain.setFreq (vdsDesc.getStartFreqs()[0],
			  vdsDesc.getEndFreqs()[vdsDesc.getNBand() - 1]);
      // Setup connections for the localWorkers and globalWorkers.
      MWConnectionSet::ShPtr workers;
      // Set up the connection for all workers.
      // Use socket connection if required, otherwise MPI connection if possible.
      // If MPI is impossible, use memory connection for a localWorker per
      // data set part.
      if (! port.empty()) {
	SocketConnectionSet* workConns (new SocketConnectionSet(port));
	workers = MWConnectionSet::ShPtr (workConns);
	workConns->addConnections (nworkers);
      } else if (nworkers > 0) {
	MPIConnectionSet* workConns (new MPIConnectionSet());
	workers = MWConnectionSet::ShPtr (workConns);
	for (int i=0; i<nworkers; ++i) {
	  // A globalWorker has MPI tag 1.
	  int tag = (i>=globalWorkerRank ? 0:1);
	  workConns->addConnection (i+1, tag);
	}
      } else {
	MemConnectionSet* workConns (new MemConnectionSet());
	workers = MWConnectionSet::ShPtr (workConns);
	for (int i=0; i<nparts; ++i) {
	  workConns->addConnection (itsFactory.create("LocalWorker"));
	}
      }
      // Find out what all remote workers can do.
      // They send a message with their capabilities after the connection is made.
      // So read from all workers and put in appropriate set.
      // It returns a vector telling which data part is handled by local worker i.
      vector<string> dataParts = setAllWorkers (*workers, nworkers);
      // Check if there are enough localWorkers.
      if (itsLocalWorkers->size() < nparts) {
	THROW(MWError, "Data Set " << itsDsDesc.getDesc().getName()
	      << " is split into " << nparts << " parts, but only "
	      << itsLocalWorkers->size() << " localWorkers are available");
      }
      // Create the master control and initialize it.
      MasterControl mc (itsLocalWorkers, itsGlobalWorkers);
      // Send the initial info.
      mc.setInitInfo (itsParSet, dataParts, fullDomain);
      // Let the the Runner execute.
      run (mc);
      // We have finished, so tell that all workers.
      mc.quit();
    }

    void ControllerBase::runLocalWorker (const string& host,
					 const string& port)
    {
      MWCOUT << "localWorker rank " << MPIConnection::getRank() << endl;
      WorkerControl pc(itsFactory.create ("LocalWorker"));
      // Connect to master on rank 0.
      if (port.empty()) {
	pc.init (MWConnection::ShPtr(new MPIConnection(0, 0)));
      } else {
	pc.init (MWConnection::ShPtr(new SocketConnection(host, port)));
      }
      pc.run();
    }

    void ControllerBase::runGlobalWorker (const string& host,
					  const string& port)
    {
      MWCOUT << "globalWorker rank " << MPIConnection::getRank() << endl;
      WorkerControl sc(itsFactory.create ("GlobalWorker"));
      // Connect to master on rank 0.
      if (port.empty()) {
	sc.init (MWConnection::ShPtr(new MPIConnection(0, 1)));
      } else {
	sc.init (MWConnection::ShPtr(new SocketConnection(host, port)));
      }
      sc.run();
    }

    vector<string> ControllerBase::setAllWorkers (MWConnectionSet& workers,
						  int nworkers)
    {
      // Create the workers description object which will be used to
      // determine which local worker to use for which data part.
      WorkersDesc workDesc((ClusterDesc(itsClusterName)));
      // If there are no remote workers (thus single process),
      // all workers are localWorkers.
      if (nworkers == 0) {
	itsLocalWorkers = workers.clone();
	string host = SocketConnection::getHostName();
	vector<int> workTypes(1,0);
	for (int i=0; i<itsLocalWorkers->size(); ++i) {
	  workDesc.addWorker (i, host, workTypes);
	}
      } else {
	// The workers are remote. Find out what they can do.
	vector<int> localInx;
	vector<int> globalInx;
	localInx.reserve (nworkers);
	// We read the info from each worker.
	BlobString buf;
	for (int i=0; i<nworkers; ++i) {
	  workers.read (i, buf);
	  WorkerInfo info = WorkerProxy::getWorkerInfo (buf);
	  if (info.getWorkType() == 0) {
	    // Remember local workers in work desc.
	    workDesc.addWorker (localInx.size(), info.getHostName(),
				info.getWorkTypes());
	    localInx.push_back(i);
	  } else {
	    globalInx.push_back(i);
	  }
	}
	itsLocalWorkers  = workers.clone (localInx);
	itsGlobalWorkers = workers.clone (globalInx);
      }
      // If there are no globalWorkers, make a globalWorker in the master process.
      if (!itsGlobalWorkers  ||  itsGlobalWorkers->size() == 0) {
	MemConnectionSet* sv = new MemConnectionSet();
	itsGlobalWorkers = MWConnectionSet::ShPtr(sv);
	sv->addConnection (itsFactory.create("GlobalWorker"));
      }
      // Now find out which data part can be handled by which local worker
      // by looking which worker has access to the data's file system.
      const vector<VdsPartDesc> parts = itsDsDesc.getParts();
      // Initialize vector telling which data part is handled by a local worker.
      vector<string> dataParts(parts.size());
      for (vector<VdsPartDesc>::const_iterator iter = parts.begin();
	   iter != parts.end();
	   ++iter) {
	int inx = workDesc.findWorker (0, iter->getFileSys());
	// We do not accept that a worker gets more than one data part.
	if (inx < 0  ||  !dataParts[inx].empty()) {
	  THROW (MWError, "No suitable worker could be found" <<
		 " to process dataset part " << iter->getName() <<
		 " on file system " << iter->getFileSys());
	}
	// Tell this worker has something to do.
	workDesc.incrLoad (inx);
	dataParts[inx] = iter->getName(); // data part i handled by worker inx
      }
      return dataParts;
    }

}} //# end namespaces
