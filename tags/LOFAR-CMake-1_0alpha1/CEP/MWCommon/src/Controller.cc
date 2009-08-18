//#  Controller.cc: Class to execute the master and workers
//#
//#  Copyright (C) 2008
//#
//#  $Id$

#include <lofar_config.h>
#include <MWCommon/Controller.h>
#include <MWCommon/MemConnectionSet.h>
#include <MWCommon/SocketConnectionSet.h>
#include <MWCommon/MPIConnectionSet.h>
#include <MWCommon/WorkerControl.h>
#include <MWCommon/VdsDesc.h>
#include <MWCommon/MWError.h>
#include <MWCommon/MWIos.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;

namespace LOFAR { namespace CEP {

    template<typename Runner>
    Controller::Controller (const Runner& runner,
			    const string& clusterName,
			    const string& logFileName)
      : itsRunner      (runner),
	itsClusterName (clusterName),
	itsLogName     (logFileName)
    {}

    int Controller::execute()
    {
      return doExecute (string(), string(), 0, 0, true);
    }

    int Controller::execute (const string& host, const string& port,
			     int nproc, int procRank)
    {
      return doExecute (host, port, nproc, procRank, false);
    }

    int Controller::doExecute (const string& host, const string& port,
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
	// or in multiple processes connected via sockets (a port is given).
	if (nnode <= 1  &&  !port.empty()) {
	  nnode = nproc;
	  rank  = procRank;
	}
	{
	  // Set the name of the output stream.
	  std::ostringstream ostr;
	  ostr << rank;
	  MWIos::setName (itsLogName + ostr.str());
	}
	// Open the parameter set and get nr of VDS parts.
	VdsDesc dsDesc(itsRunner.getDataSetName());
	int nparts = dsDesc.getParts().size();
	// Find out if this process is master, globalWorker, or localWorker.
	int globalWorkerRank = 0;
	if (nnode > nparts+1) {
	  // The master and globalWorker can run on different nodes.
	  globalWorkerRank = 1;
	}
	// Initialize and run the controls.
	if (rank == 0) {
	  runMaster (dsDesc, port, globalWorkerRank, nnode-1, nparts);
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

    void Controller::runMaster (const VdsDesc& dsDesc,
				const string& port,
				int globalWorkerRank,
				int nworkers, int nparts)
    {
      // Get the full observation domain for this data set.
      const VdsPartDesc& vdsDesc = dsDesc.getDesc();
      ObsDomain fullDomain;
      fullDomain.setTime (vdsDesc.getStartTime(),
			  vdsDesc.getEndTime());
      fullDomain.setFreq (vdsDesc.getStartFreqs()[0],
			  vdsDesc.getEndFreqs()[vdsDesc.getNBand() - 1]);
      // Setup connections for the localWorkers and globalWorkers.
      MWConnectionSet::ShPtr workers;
      // Set up the connection for all workers.
      // Use socket connection if required, otherwise MPI connection if possible.
      // If MPI is impossible, use memory connection for a localWorker per VDS part.
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
      setAllWorkers (*workers, nworkers);
      // Check if there are enough localWorkers.
      if (itsLocalWorkers->size() < nparts) {
	THROW(MWError, "The Visibility Data Set is split into "
	      << nparts << " parts, so mwcontrol has to have at least "
	      << nparts+1 << " localWorkers, but only "
	      << itsLocalWorkers->size() << " are available");
      }
      // Create the master control and initialize it.
      MasterControl mc (itsLocalWorkers, itsGlobalWorkers);
      // Send the initial info.
      mc.setInitInfo (itsRunner.getParSet(), fullDomain);
      // Let the caller execute.
      itsRunner.run (mc);
      mc.quit();
    }

    void Controller::runLocalWorker (const string& host, const string& port)
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

    void Controller::runGlobalWorker (const string& host, const string& port)
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

    void Controller::setAllWorkers (MWConnectionSet& workers, int nworkers)
    {
      // If there are no remote workers, just copy the workers to localWorkers.
      if (nworkers == 0) {
	itsLocalWorkers = workers.clone();
      } else {
	vector<int> localInx;
	vector<int> globalInx;
	localInx.reserve (nworkers);
	// We have to read from every worker and see what it can do.
	BlobString buf;
	for (int i=0; i<nworkers; ++i) {
	  workers.read (i, buf);
	  WorkerInfo info = WorkerProxy::getWorkerInfo (buf);
	  if (info.getWorkType() == 0) {
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
    }

}} //# end namespaces
