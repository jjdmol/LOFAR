//#  BDBReplicator.cc: Handle replication of a Berkeley DB database
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

// General includes
#include <Common/LofarLogger.h>
#include <iostream>

// Application specific includes
#include <BDBReplicator.h>

namespace LOFAR {
namespace BDBReplication {
using namespace std;

BDBReplicator::BDBReplicator(const string& DbEnvName,
			     const string& hostName, 
			     const int port,
			     const string& masterHostName,
			     const int masterPort,
			     const bool master)
  : itsReplicationStarted(false),
    itsDbEnvName(DbEnvName),
    itsHostName(hostName),
    itsPort(port),
    itsMasterHostName(masterHostName),
    itsMasterPort(masterPort),
    itsIsMaster(master),
    itsConnector(hostName, port, itsSiteMap),
    itsConnectorThread(0),
    itsCommunicator(itsConnector, itsSiteMap),
    itsCommunicatorThread(0),
    itsDbEnv(0)
{
  if (master){
    LOG_TRACE_FLOW_STR("master "<<DbEnvName<<" "<<hostName<<" "<<port<<" "<<masterHostName<<" "<<masterPort);
  } else {
    LOG_TRACE_FLOW_STR("slave "<<DbEnvName<<" "<<hostName<<" "<<port<<" "<<masterHostName<<" "<<masterPort);
  };
}

void BDBReplicator::startReplication()
{
#define BDBREPL_USE_THREADS
  if (!itsReplicationStarted) {
#ifdef BDBREPL_USE_THREADS
    itsConnectorThread = new boost::thread(itsConnector);
    // wait until the listener is listening
    while(!itsConnector.isListening()) {
      LOG_TRACE_FLOW_STR("BDBReplicator waiting for listener");
      sleep(1);
    }
#else
    while(!itsConnector.isListening()) {
      itsConnector.listenOnce();
      LOG_TRACE_FLOW_STR("BDBReplicator waiting for listener");
    }
    while(itsConnector.listenOnce());
#endif

    if(!itsIsMaster) {
      // try to connect to master
      while (!itsConnector.connectTo(itsMasterHostName.c_str(), itsMasterPort)) {
	LOG_ERROR_STR("BDBReplicator trying to connect to master on port "<<itsMasterPort);
	sleep(1);
      }
    }
    
    LOG_TRACE_FLOW("BDBReplicator starting replication");
    itsDbEnv = new BDBEnv(DB_CXX_NO_EXCEPTIONS, itsSiteMap);
    //  LOG_TRACE_FLOW("BDBReplicator setting rep_transport");
    ASSERTSTR (itsDbEnv->set_rep_transport(1, &BDBCommunicator::send) == 0, "cannot set rep transport in BDBReplicator");
    u_int32_t flags = DB_CREATE | DB_THREAD | DB_PRIVATE | DB_INIT_REP |
      DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
    LOG_TRACE_FLOW_STR("BDBReplicator opening environment: "<<itsDbEnvName.c_str());
    int ret = itsDbEnv->open(itsDbEnvName.c_str(), flags, 0);
    ASSERTSTR(ret == 0, "Cannot open db environment: "<<itsDbEnv->strerror(ret));
    
    // set connection data
    itsPort = itsConnector.getPort();
    char *buffer = new char[sizeof(int)+itsHostName.size() + 2];
    memset(buffer, 0, sizeof(int)+itsHostName.size() + 2);
    memcpy(buffer, &itsPort, sizeof(int));
    strncpy(&buffer[sizeof(int)], itsHostName.c_str(), itsHostName.size());
    Dbt connectionData(buffer, sizeof(int)+itsHostName.size() + 2);

    if (itsIsMaster) {
      flags = DB_REP_MASTER;
    } else {
      flags = DB_REP_CLIENT;
    };
    //  LOG_TRACE_FLOW("BDBReplicator executing rep_start");
    ASSERTSTR(itsDbEnv->rep_start(&connectionData, flags) == 0, "could not execute rep start in BDBReplicator");
    
    itsCommunicator.setEnv(itsDbEnv);
    
    //  LOG_TRACE_FLOW("BDBReplicator starting threads");
#ifdef BDBREPL_USE_THREADS
    itsCommunicatorThread = new boost::thread(itsCommunicator);
#else
    handleMessages();
#endif
    LOG_TRACE_FLOW("BDBReplicator replication started");  

//     while (!itsIsMaster && !itsCommunicator.isStartupDone()) {
//       LOG_ERROR_STR("BDBReplicator waiting until startup is done");
//       sleep(1);
//     }
  }
  itsReplicationStarted = true;
}

BDBReplicator::~BDBReplicator()
{
  LOG_TRACE_FLOW("BDBReplicator destructor");
#ifdef BDBREPL_USE_THREADS
  itsConnector.stop();
  itsCommunicator.stop();

  
  itsConnectorThread->join();
  itsCommunicatorThread->join();

  delete itsConnectorThread;
  delete itsCommunicatorThread;
#else
  handleMessages();
#endif

  if (itsDbEnv !=0) {
    itsDbEnv->close(0);
    delete itsDbEnv;
  }

  boost::mutex::scoped_lock sml(itsSiteMap.itsMutex);
  // itsSiteMap.lock();
  BDBSiteMap::iterator it;
  for (it=itsSiteMap.begin(); it!=itsSiteMap.end(); it++) {
    delete it->second;      
  }
  //  itsSiteMap.unlock();
} 

void BDBReplicator::handleMessages() 
{
#ifndef BDBREPL_USE_THREADS
  bool notReady = true;
  while(notReady) {
    // do this in seperate lines, because the part after the ||
    // may not be called if the part before is true
    notReady = itsConnector.listenOnce();
    notReady = itsCommunicator.listenOnce() || notReady; 
    boost::thread::yield();
  }
#endif
}
}
}
