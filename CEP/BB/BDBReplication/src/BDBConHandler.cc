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

boost::mutex BDBCHThread::theirSiteMapMutex;
map<int, BDBSite*> BDBCHThread::theirSiteMap;
int BDBCHThread::theirObjectCounter = 0;
bool BDBCHThread::theirShouldStop = false;
int BDBCHThread::theirLastEnvId = 2;

BDBCHThread::BDBCHThread(const string& hostName, const int port)
  : itsEnvId(1),
    itsDbEnv(0),
    itsHostName(hostName),
    itsPort(port)
{
  theirObjectCounter++;
};
BDBCHThread::BDBCHThread(const BDBCHThread& other)
  : itsEnvId(other.itsEnvId),
    itsDbEnv(other.itsDbEnv),
    itsHostName(other.itsHostName),
    itsPort(other.itsPort)
{
  theirObjectCounter++;
};

BDBCHThread::~BDBCHThread()
{
  theirObjectCounter--;
  if (theirObjectCounter == 0) {
    map<int, BDBSite*>::iterator it;
    for (it=theirSiteMap.begin(); it!=theirSiteMap.end(); it++) {
      delete it->second->getSocket();
      delete it->second;      
    }
  }
}
void BDBCHThread::stop()
{
  theirShouldStop = true;
  LOG_TRACE_FLOW("BDBConHandlThread stop set");
};

bool BDBCHThread::shouldStop()
{
  return theirShouldStop;
};

void BDBCHThread::operator()()
{
  LOG_TRACE_FLOW("BDBConHandlThread started");
  while(!shouldStop()) {
    map<int, BDBSite*>::iterator it;
    for (it=theirSiteMap.begin(); it!=theirSiteMap.end(); it++) {
      char* cBuffer = 0;
      char* rBuffer = 0;
      int messageSize = 0;
      Dbt rec, control;
      bool isMessagePresent = false;
      { // new scope because of scoped lock
	boost::mutex::scoped_lock sl(theirSiteMapMutex);

	Socket* mySocket = it->second->getSocket();
	// check if there is data available
	// this loop could use more error checking
	if (mySocket->read(&messageSize, 4) == 4) {
	  //cout<<"Received message from env "<<it->first<<endl;
	  LOG_TRACE_FLOW_STR("read size:" <<messageSize);
	  LOG_TRACE_FLOW("BDBConHandlThread data present on socket");
	  isMessagePresent = true;
	  // read control Dbt
	  cBuffer = new char[messageSize];
	  if (messageSize > 0) {
	    mySocket->readBlocking(cBuffer, messageSize);
	  }
	  control.set_data(cBuffer);
	  control.set_size(messageSize);
	  
	  // read rec Dbt
	  mySocket->readBlocking(&messageSize, 4);
	  LOG_TRACE_FLOW_STR("read size:" <<messageSize);
	  rBuffer = new char[messageSize];
	  if (messageSize > 0) {
	    mySocket->readBlocking(rBuffer, messageSize);
	  }
	  rec.set_data(rBuffer);
	  rec.set_size(messageSize);
	};
      }
      if (isMessagePresent) {
	handleMessage(&rec, &control, it->first);
      }
      delete [] cBuffer;
      delete [] rBuffer;
    }
    // TODO: remove the sleep here
    //            sleep(10);
    boost::thread::yield();
  }
}

int BDBCHThread::send(DbEnv *dbenv,
		      const Dbt *control, 
		      const Dbt *rec, 
		      const DbLsn *lsnp,
		      int envid, 
		      u_int32_t flags)
{
  LOG_TRACE_FLOW("BDBConHandlThread send function");
  // buffer or not

  boost::mutex::scoped_lock sl(theirSiteMapMutex);
  if (envid == DB_EID_BROADCAST) {
    map<int, BDBSite*>::iterator it;
    for (it=theirSiteMap.begin(); it!=theirSiteMap.end(); it++)
    {
      sendOne(control, rec, it->second->getSocket());
    }
  } else {
    map<int, BDBSite*>::iterator pos;
    pos = theirSiteMap.find(envid);
    if (pos != theirSiteMap.end()){
      sendOne(control, rec, pos->second->getSocket());
    } else {
      cerr<<"BDB trying to send to unknown environment"<<endl;
    }
  }
  LOG_TRACE_FLOW("BDBConHandlThread send finished");
  return 0;
}

void BDBCHThread::sendOne(const Dbt *control, 
			 const Dbt *rec, 
			 Socket* mySocket)
{
  //  LOG_TRACE_FLOW("BDBConHandlThread sending one");
  int size = control->get_size();
  LOG_TRACE_FLOW_STR("writing size:" <<size);
  mySocket->write(&size, 4);
  if (control->get_size() > 0)
    mySocket->write(control->get_data(), control->get_size());
  size = rec->get_size();
  LOG_TRACE_FLOW_STR("writing size:" <<size);
  mySocket->write(&size, 4);
  if (rec->get_size() > 0)
    mySocket->write(rec->get_data(), rec->get_size());
  //  LOG_TRACE_FLOW("BDBConHandlThread sending one");
}


void BDBCHThread::connectTo(const char* hostName, const int port)
{
  Socket* newSocket = new Socket();
  BDBSite* newSite = new BDBSite(hostName, port, newSocket);
  LOG_TRACE_FLOW_STR("BDBConHandlThread connecting to "<<*newSite);
  LOG_TRACE_FLOW_STR("BDBConHandlThread connecting from "<<itsHostName<<":"<<itsPort);
  cout<<"BDBConHandlThread connecting to "<<*newSite<<endl;

  char service[20];
  snprintf(service, 20, "%d", port);
  newSocket->initClient(hostName, service);
  if (newSocket->connect(0) == Socket::SK_OK) {
    // send my connection data
    newSocket->write(&itsPort,4);
    int messageSize = itsHostName.size();
    newSocket->write(&messageSize, 4);
    newSocket->write(itsHostName.c_str(), messageSize);
    
    addSite(newSite);
  };
  //  LOG_TRACE_FLOW("BDBConHandlThread connectedTo");
}

void BDBCHThread::handleMessage(Dbt* rec, Dbt* control, int envId)
{
  LOG_TRACE_FLOW("BDBConHandlThread handling message");

  LOG_TRACE_FLOW_STR(control<<" "<<rec<<" "<<envId<<" "<<itsDbEnv);

  DbLsn retLSN;
  //  cout<<"handling message ...";
  int ret = itsDbEnv->rep_process_message(control, rec, &envId, &retLSN);
  //  cout<<"ready"<<endl;
  
  int port = 0;
  char* hostName = NULL;
  switch (ret) {
//   case DB_REP_STARTUPDONE:
//     LOG_TRACE_FLOW("BDBConHandlThread startup done");
//     cerr<<"startup done"<<endl;
//     break;

  case DB_REP_NEWSITE:
    LOG_TRACE_FLOW("BDBConHandlThread new site detected");
    port = *((int*)rec->get_data());
    hostName =(char*)rec->get_data()+4;

    cout<<"new site detected by libdb: "<<hostName<<":"<<port<<endl;
    connectTo(hostName, port);    
    break;
    
  case DB_REP_DUPMASTER:
    cerr<<"More than one master"<<endl;
    exit(1);
    break;

  case DB_REP_HOLDELECTION:
    cerr<<"hold election"<<endl;
    exit(1);    
    break;

  case DB_REP_ISPERM:
    cerr<<"Permanent message no "<<endl;
    break;

  case DB_REP_NEWMASTER:
    cerr<<"new master"<<endl;
    break;

  case DB_REP_NOTPERM:
    cerr<<"Non-permanent message no "<<endl;
    break;
  
  default:
    if (ret != 0) 
      cerr<<"error from handle message: "<<itsDbEnv->strerror(ret)<<endl;

  }
  LOG_TRACE_FLOW("message handled");
}

void BDBCHThread::setEnv(DbEnv* dbenv)
{
  //  LOG_TRACE_FLOW_STR("Setting db environment to "<<dbenv);
  itsDbEnv = dbenv;
};

void BDBCHThread::addSite(BDBSite* newSite)
{
  bool found = false;
  map<int, BDBSite*>::iterator it;
  {
    boost::mutex::scoped_lock sl(theirSiteMapMutex);
    for (it = theirSiteMap.begin(); it!=theirSiteMap.end(); it++) {
      if (*(it->second) == *(newSite)) found = true;
    }
    if (!found) {
      theirSiteMap[theirLastEnvId++] = newSite;
      LOG_TRACE_FLOW_STR("New site "<<*newSite);
      cout<<"site added: "<<*newSite<<endl;
    } else {
      cout<<"site already exists: "<<*newSite<<endl;
      LOG_TRACE_FLOW_STR("Site "<<*newSite<<"already exists");
      delete newSite;
    };
  }
  printSiteMap();
}

void BDBCHThread::printSiteMap()
{
  cout<<endl
      <<"    SITEMAP" << endl
      <<"================"<<endl<<endl;

  map<int, BDBSite*>::iterator it;
  boost::mutex::scoped_lock sl(theirSiteMapMutex);
  for (it = theirSiteMap.begin(); it!=theirSiteMap.end(); it++) {
    cout<<it->first<<": "<<*(it->second)<<endl;
  }
}
