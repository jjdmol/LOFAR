//#  BDBCommunicator.cc: Handle sending and receiveing messages
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
#include <BDBCommunicator.h>

using namespace LOFAR;
using namespace std;

boost::mutex BDBCommunicator::theirSiteMapMutex;
map<int, BDBSite*> BDBCommunicator::theirSiteMap;
int BDBCommunicator::theirObjectCounter = 0;
bool BDBCommunicator::theirShouldStop = false;
int BDBCommunicator::theirLastEnvId = 2;

BDBCommunicator::BDBCommunicator(const string& hostName)
  : itsEnvId(1),
    itsDbEnv(0),
    itsHostName(hostName),
    itsPort(0)
{
  theirObjectCounter++;
};
BDBCommunicator::BDBCommunicator(const BDBCommunicator& other)
  : itsEnvId(other.itsEnvId),
    itsDbEnv(other.itsDbEnv),
    itsHostName(other.itsHostName),
    itsPort(other.itsPort)
{
  theirObjectCounter++;
};

BDBCommunicator::~BDBCommunicator()
{
  theirObjectCounter--;
  if (theirObjectCounter == 0) {
    map<int, BDBSite*>::iterator it;
    for (it=theirSiteMap.begin(); it!=theirSiteMap.end(); it++) {
      delete it->second;      
    }
  }
}
void BDBCommunicator::stop()
{
  theirShouldStop = true;
  LOG_TRACE_FLOW("BDBConHandlThread stop set");
};

bool BDBCommunicator::shouldStop()
{
//   if (theirShouldStop) {
//     LOG_TRACE_FLOW("conhandler should stop");
//   } else {
//     LOG_TRACE_FLOW("conhandler should not stop");
//   }
  return theirShouldStop;
};

void BDBCommunicator::operator()()
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

	BDBSite& mySite = *(it->second);
	// check if there is data available
	// this loop could use more error checking
	if (mySite.recv(&messageSize, 4) == 4) {
	  //LOG_TRACE_FLOW("Received message from env "<<it->first);
	  LOG_TRACE_FLOW_STR("read size:" <<messageSize);
	  LOG_TRACE_FLOW("BDBConHandlThread data present on socket");
	  isMessagePresent = true;
	  // read control Dbt
	  cBuffer = new char[messageSize];
	  if (messageSize > 0) {
	    mySite.recv(cBuffer, messageSize);
	  }
	  control.set_data(cBuffer);
	  control.set_size(messageSize);
	  
	  // read rec Dbt
	  mySite.recv(&messageSize, 4);
	  LOG_TRACE_FLOW_STR("read size:" <<messageSize);
	  rBuffer = new char[messageSize];
	  if (messageSize > 0) {
	    mySite.recv(rBuffer, messageSize);
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

int BDBCommunicator::send(DbEnv *dbenv,
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
      sendOne(control, rec, *(it->second));
    }
  } else {
    map<int, BDBSite*>::iterator pos;
    pos = theirSiteMap.find(envid);
    if (pos != theirSiteMap.end()){
      sendOne(control, rec, *(pos->second));
    } else {
      LOG_ERROR("BDB trying to send to unknown environment");
    }
  }
  LOG_TRACE_FLOW("BDBConHandlThread send finished");
  return 0;
}

void BDBCommunicator::sendOne(const Dbt *control, 
			 const Dbt *rec, 
			 BDBSite& mySite)
{
  //  LOG_TRACE_FLOW("BDBConHandlThread sending one");
  int size = control->get_size();
  LOG_TRACE_FLOW_STR("writing size:" <<size);
  mySite.send(&size, 4);
  if (control->get_size() > 0)
    mySite.send(control->get_data(), control->get_size());
  size = rec->get_size();
  LOG_TRACE_FLOW_STR("writing size:" <<size);
  mySite.send(&size, 4);
  if (rec->get_size() > 0)
    mySite.send(rec->get_data(), rec->get_size());
  //  LOG_TRACE_FLOW("BDBConHandlThread sending one");
}


bool BDBCommunicator::connectTo(const char* hostName, const int port)
{
  if (itsPort == 0) return false;
  Socket* newSocket = new Socket();
  BDBSite* newSite = new BDBSite(hostName, port, newSocket);
  LOG_TRACE_FLOW_STR("BDBConHandlThread connecting to "<<*newSite);
  LOG_TRACE_FLOW_STR("BDBConHandlThread connecting from "<<itsHostName<<":"<<itsPort);
  LOG_TRACE_FLOW_STR("BDBConHandlThread connecting to "<<*newSite);

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
    return true;
  } else {
    return false;
  }
}

void BDBCommunicator::handleMessage(Dbt* rec, Dbt* control, int envId)
{
  LOG_TRACE_FLOW("BDBConHandlThread handling message");

  LOG_TRACE_FLOW_STR(control<<" "<<rec<<" "<<envId<<" "<<itsDbEnv);

  DbLsn retLSN;
  //  LOG_TRACE_FLOW("handling message ...";
  int ret = itsDbEnv->rep_process_message(control, rec, &envId, &retLSN);
  //  LOG_TRACE_FLOW("ready");
  
  int port = 0;
  char* hostName = NULL;
  switch (ret) {
//   DB_REP_STARTUPDNE is not supported by older versions of libdb
//   case DB_REP_STARTUPDONE:
//     LOG_TRACE_FLOW("BDBConHandlThread startup done");
//     break;

  case DB_REP_NEWSITE:
    LOG_TRACE_FLOW("BDBConHandlThread new site detected");
    port = *((int*)rec->get_data());
    hostName =(char*)rec->get_data()+4;

    LOG_TRACE_FLOW_STR("new site detected by libdb: "<<hostName<<":"<<port);
    connectTo(hostName, port);    
    break;
    
  case DB_REP_DUPMASTER:
    ASSERTSTR(false, "More than one master");
    break;

  case DB_REP_HOLDELECTION:
    ASSERTSTR(false, "hold election");
    break;

  case DB_REP_ISPERM:
    LOG_TRACE_FLOW("Permanent message no ");
    break;

  case DB_REP_NEWMASTER:
    LOG_TRACE_FLOW("new master");
    break;

  case DB_REP_NOTPERM:
    LOG_TRACE_FLOW("Non-permanent message no ");
    break;
  
  default:
    if (ret != 0) 
      LOG_ERROR_STR("error from handle message: "<<itsDbEnv->strerror(ret));

  }
  LOG_TRACE_FLOW("message handled");
}

void BDBCommunicator::setEnv(DbEnv* dbenv)
{
  //  LOG_TRACE_FLOW_STR("Setting db environment to "<<dbenv);
  itsDbEnv = dbenv;
};

void BDBCommunicator::addSite(BDBSite* newSite)
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
      LOG_TRACE_FLOW_STR("site added: "<<*newSite);
    } else {
      LOG_TRACE_FLOW_STR("site already exists: "<<*newSite);
      LOG_TRACE_FLOW_STR("Site "<<*newSite<<"already exists");
      delete newSite;
    };
  }
  printSiteMap();
}

void BDBCommunicator::printSiteMap()
{
  LOG_TRACE_FLOW_STR(endl
      <<"    SITEMAP" << endl
      <<"================"<<endl);

  map<int, BDBSite*>::iterator it;
  boost::mutex::scoped_lock sl(theirSiteMapMutex);
  for (it = theirSiteMap.begin(); it!=theirSiteMap.end(); it++) {
    LOG_TRACE_FLOW_STR(it->first<<": "<<*(it->second));
  }
}
