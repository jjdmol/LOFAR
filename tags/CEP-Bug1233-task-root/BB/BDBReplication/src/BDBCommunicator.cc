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

namespace LOFAR{
  namespace BDBReplication{
using namespace std;

BDBCommunicatorRep::BDBCommunicatorRep(const BDBConnector& connector, BDBSiteMap& siteMap, BDBSyncer& syncer)
  : itsReferences(0),
    itsShouldStop(false),
    itsStartupDone(false),
    itsConnector(connector),
    itsSyncer(syncer),
    itsSiteMap(siteMap),
    itsDbEnv(0)
{
};

BDBCommunicatorRep::~BDBCommunicatorRep()
{
}

void BDBCommunicatorRep::stop()
{
  itsShouldStop = true;
  LOG_TRACE_FLOW("BDBConHandlThread stop set");
};

bool BDBCommunicatorRep::shouldStop()
{
  return itsShouldStop;
};


void BDBCommunicatorRep::operator()()
{
  LOG_TRACE_FLOW("BDBConHandlThread started");
  while(!shouldStop()) {
    if (!listenOnce()) 
      boost::thread::yield();
  }
};

bool BDBCommunicatorRep::listenOnce() {
  vector<BDBMessage*> messages;
  bool isMessagePresent = false;
  { 
    boost::mutex::scoped_lock sml(itsSiteMap.itsMutex);
    BDBSiteMap::iterator it;
    for (it=itsSiteMap.begin(); it!=itsSiteMap.end(); it++) {
      BDBSite& mySite = *(it->second);
      LOG_TRACE_FLOW_STR("trying to read from:" <<mySite);
      {
	boost::mutex::scoped_lock sml(mySite.itsMutex);
	BDBMessage* message = new BDBMessage(BDBMessage::TO_BE_RECEIVED);
	if (mySite.recv(*message)) {
	  message->setEnvId(it->first);
	  messages.push_back(message);
	}
      }
    }
  }
  vector<BDBMessage*>::iterator mit;
  for(mit = messages.begin(); mit != messages.end(); mit++) {
    switch ((*mit)->getType()) {
    case BDBMessage::LIBBDB:
      handleMessage(**mit);
      break;
    case BDBMessage::STARTUP_DONE:
      itsStartupDone = true;
      LOG_TRACE_FLOW("Startup done received on slave");
      break;
    case BDBMessage::SYNC_REPLY:
      itsSyncer.handleReply(**mit);
      break;
    default:
      break;
    }
    delete *mit;
  };
  SyncReqType sr = itsSyncer.handleRequest();
  if (sr > 0) {
    BDBMessage syncreply(BDBMessage::SYNC_REPLY);
    syncreply.setSyncRequestNumber(sr);
    
    boost::mutex::scoped_lock sml(itsSiteMap.itsMutex);
    BDBSiteMap::iterator it;
    for (it=itsSiteMap.begin(); it!=itsSiteMap.end(); it++)
    {
      LOG_TRACE_FLOW_STR("Sending syncreply to site " << it->first);
      it->second->lock();
      it->second->send(syncreply);
      it->second->unlock();
    }
  }  
  return messages.size()>0;
}

void BDBCommunicatorRep::handleMessage(BDBMessage& message)
{
  Dbt& rec = message.getRec();
  Dbt& control = message.getControl();
  int envId = message.getEnvId();
  LOG_TRACE_FLOW("BDBConHandlThread handling message");

  DbLsn retLSN;
  //  LOG_TRACE_FLOW("handling message ...";
  int ret = itsDbEnv->rep_process_message(&control, &rec, &envId, &retLSN);
  //  LOG_TRACE_FLOW("ready");
  
  int port = 0;
  char* hostName = NULL;
  switch (ret) {
  case DB_REP_STARTUPDONE:
    LOG_TRACE_FLOW("BDBConHandlThread startup done");
    //itsStartupDone = true;
    break;

  case DB_REP_NEWSITE:
    LOG_TRACE_FLOW("BDBConHandlThread new site detected");
    port = *((int*)rec.get_data());
    hostName =(char*)rec.get_data()+4;

    LOG_TRACE_FLOW_STR("new site detected by libdb: "<<hostName<<":"<<port);
    itsConnector.connectTo(hostName, port);    
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

void BDBCommunicatorRep::setEnv(DbEnv* dbenv)
{
  //  LOG_TRACE_FLOW_STR("Setting db environment to "<<dbenv);
  itsDbEnv = dbenv;
};

void BDBCommunicatorRep::sendStartupDone() {
  itsStartupDone = true;

  BDBEnv& myEnv = (BDBEnv&) *itsDbEnv;
  myEnv.AssertCorrectInstance();
  BDBSiteMap& mySiteMap = myEnv.itsSiteMap;

  boost::mutex::scoped_lock sml(mySiteMap.itsMutex);
  BDBSiteMap::iterator it;
  BDBMessage suMes(BDBMessage::STARTUP_DONE);
  for (it=mySiteMap.begin(); it!=mySiteMap.end(); it++)
  {
    LOG_TRACE_FLOW_STR("Sending startup done to site " << it->first);
    it->second->lock();
    it->second->send(suMes);
    it->second->unlock();
  }
}

bool BDBCommunicatorRep::isStartupDone()
{ return itsStartupDone;};


BDBCommunicator::BDBCommunicator(const BDBConnector& connector, BDBSiteMap& siteMap, BDBSyncer& syncer)
{
  itsRep = new BDBCommunicatorRep(connector, siteMap, syncer);
  itsRep->itsReferences++;
}
BDBCommunicator::BDBCommunicator(const BDBCommunicator& other)
  : itsRep(other.itsRep)
{
  ASSERTSTR(itsRep != 0, "Invalid BDBCommunicatorRep");
  itsRep->itsReferences++;
}
BDBCommunicator::~BDBCommunicator()
{
  itsRep->itsReferences--;
  if (itsRep->itsReferences == 0)
    delete itsRep;
}
void BDBCommunicator::setEnv(DbEnv* DbEnv)
{
  itsRep->setEnv(DbEnv);
}
void BDBCommunicator::stop()
{
  itsRep->stop();
}
void BDBCommunicator::operator()()
{
  itsRep->operator()();
}
bool BDBCommunicator::listenOnce() {
  itsRep->listenOnce();
}

void BDBCommunicator::sendStartupDone() {
  itsRep->sendStartupDone();
}

bool BDBCommunicator::isStartupDone()
{
  return itsRep->isStartupDone();
}
  
int BDBCommunicator::send(DbEnv *dbenv,
		      const Dbt *control, 
		      const Dbt *rec, 
		      const DbLsn *lsnp,
		      int envid, 
		      u_int32_t flags)
{
  //  LOG_TRACE_FLOW("BDBConHandlThread send function");
  // buffer or not

  BDBEnv& myEnv = (BDBEnv&) *dbenv;
  myEnv.AssertCorrectInstance();
  BDBSiteMap& mySiteMap = myEnv.itsSiteMap;

  BDBMessage myMessage(BDBMessage::LIBBDB);
  myMessage.setRec(*rec);
  myMessage.setControl(*control);
  myMessage.setEnvId(envid);

  if (envid == DB_EID_BROADCAST) {
    boost::mutex::scoped_lock sml(mySiteMap.itsMutex);
    //    mySiteMap.lock();
    BDBSiteMap::iterator it;
    for (it=mySiteMap.begin(); it!=mySiteMap.end(); it++)
    {
      LOG_TRACE_FLOW_STR("Sending(bc) to site " << it->first);
      it->second->lock();
      it->second->send(myMessage);
      it->second->unlock();
    }
    //    mySiteMap.unlock();
  } else {
    boost::mutex::scoped_lock sml(mySiteMap.itsMutex);
    //    mySiteMap.lock();
    BDBSite& dstSite = mySiteMap.find(envid);
    boost::mutex::scoped_lock sl(dstSite.itsMutex);
    //    dstSite.lock();
    dstSite.send(myMessage);
    //    dstSite.unlock();
    //    mySiteMap.unlock();
  }
  //  LOG_TRACE_FLOW("BDBConHandlThread send finished");
  return 0;
}
}
}
