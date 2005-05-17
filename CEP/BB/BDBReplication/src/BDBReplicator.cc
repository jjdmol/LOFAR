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

using namespace LOFAR;
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
    itsCHThreadObject(hostName, port),
    itsCHThread(0),
    itsLThreadObject(port, &itsCHThreadObject),
    itsLThread(0),
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
  if (!itsReplicationStarted) {
    itsLThread = new boost::thread(itsLThreadObject);

    if(!itsIsMaster) {
      sleep(1);
      itsCHThreadObject.connectTo(itsMasterHostName.c_str(), itsMasterPort);
      sleep(1);
    }
    
    LOG_TRACE_FLOW("BDBReplicator starting replication");
    itsDbEnv = new DbEnv(DB_CXX_NO_EXCEPTIONS);
    //  LOG_TRACE_FLOW("BDBReplicator setting rep_transport");
    if (itsDbEnv->set_rep_transport(1, &BDBCHThread::send) !=0 )
      LOG_TRACE_FLOW("cannot set rep transport in BDBReplicator");
    u_int32_t flags = DB_CREATE | DB_THREAD | DB_INIT_REP |
      DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
    LOG_TRACE_FLOW_STR("BDBReplicator opening environment: "<<itsDbEnvName.c_str());
    if (itsDbEnv->open(itsDbEnvName.c_str(), flags, 0) != 0) {
      LOG_TRACE_FLOW("Cannot open db environment");
    }
    
    // set connection data
    char *buffer = new char[sizeof(int)+itsHostName.size() + 2];
    memset(buffer, 0, sizeof(int)+itsHostName.size() + 2);
    memcpy(buffer, &itsPort, sizeof(int));
    strcpy(buffer+sizeof(int), itsHostName.c_str());
    Dbt connectionData(buffer, sizeof(int)+itsHostName.size() + 2);
    if (itsIsMaster) {
      flags = DB_REP_MASTER;
    } else {
      flags = DB_REP_CLIENT;
    };
    //  LOG_TRACE_FLOW("BDBReplicator executing rep_start");
    if (itsDbEnv->rep_start(&connectionData, flags) != 0)
      LOG_TRACE_FLOW ("could not execute rep start in BDBReplicator");
    
    itsCHThreadObject.setEnv(itsDbEnv);
    
    //  LOG_TRACE_FLOW("BDBReplicator starting threads");
    itsCHThread = new boost::thread(itsCHThreadObject);
    LOG_TRACE_FLOW("BDBReplicator replication started");  

    if (!itsIsMaster) {
    // give master and client some time to connect
    sleep(3);
    }
  }
  itsReplicationStarted = true;
}

BDBReplicator::~BDBReplicator()
{
  //  LOG_TRACE_FLOW("BDBReplicator destructor");
  itsLThreadObject.stop();
  itsLThread->join();
  delete itsLThread;
  itsCHThreadObject.stop();
  itsCHThread->join();
  delete itsCHThread;
  if (itsDbEnv !=0) {
    itsDbEnv->close(0);
    delete itsDbEnv;
  }
} 
