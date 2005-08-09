//#  BDBConnector.cc: Handle replication of a Berkeley DB database
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
#include <boost/thread.hpp>

// Application specific includes
#include <BDBConnector.h>

using namespace LOFAR;
using namespace std;

BDBConnectorRep::BDBConnectorRep(const string hostName,
				 const int port,
				 BDBSiteMap& siteMap)
  :itsSiteMap(siteMap),
   itsHostName(hostName),
   itsPort(port),
   itsShouldStop(false),
   itsIsListening(false),
   itsReferences(0),
   itsListenSocket(0)
{
  //  LOG_TRACE_FLOW("BDBConnector constructor");
}

BDBConnectorRep::~BDBConnectorRep()
{
  //  LOG_TRACE_FLOW("BDBConnector detor");
}

void BDBConnectorRep::stop()
{
  itsShouldStop = true;
};

bool BDBConnectorRep::shouldStop()
{
//   if (itsShouldStop) {
//     cout<<"listener should stop"<<endl;
//   } else {
//     cout<<"listener should not stop"<<endl;
//   }
  return itsShouldStop;
};

bool BDBConnectorRep::connectTo(string hostName, int port) const
{
  if (itsPort == 0) return false;
  char service[20];
  snprintf(service, 20, "%d", port);
  LOG_TRACE_FLOW_STR("BDBConnector connecting to "<<hostName<<":"<<port);
  LOG_TRACE_FLOW_STR("BDBConnector connecting from "<<itsHostName<<":"<<itsPort);
  Socket* newSocket = new Socket((string("outgoing_to_") + hostName).c_str(),
				 hostName.c_str(),
				 service,
				 Socket::TCP);
  newSocket->setBlocking(false);
  BDBSite* newSite = new BDBSite(hostName.c_str(), port, newSocket);

  if (newSocket->connect(0) == Socket::SK_OK) {
    // send my connection data
    newSocket->writeBlocking(&itsPort,4);
    int messageSize = itsHostName.size();
    newSocket->writeBlocking(&messageSize, 4);
    newSocket->writeBlocking(itsHostName.c_str(), messageSize);
    
    itsSiteMap.addSite(newSite);
    return true;
  } else {
    delete newSite;
    return false;
  }
}


void BDBConnectorRep::operator()()
{
  LOG_TRACE_FLOW("BDBConnector starting thread");
  while (!shouldStop()) {
    if (!listenOnce())
      boost::thread::yield();
  }
};

bool BDBConnectorRep::listenOnce(){
  if (itsListenSocket == 0) {
    itsListenSocket = new Socket("Listen socket of Connector");
    char service[20];
    snprintf(service, 20, "%d", itsPort);
    LOG_TRACE_FLOW_STR("BDBConnector initting server on port "<<service);
    while (!itsIsListening) {    
      char service[20];
      snprintf(service, 20, "%d", itsPort);
      if (itsListenSocket->initServer(service) != Socket::SK_OK) {
	LOG_TRACE_FLOW_STR("Could not init server ("<<itsListenSocket->errstr()<<"), retrying");
	itsPort++;
      } else {
	itsIsListening = true;
      }
    }
  }

  Socket* newSocket = itsListenSocket->accept(100); // wait 500 ms for new connection
  if (newSocket != 0){
    int port=0;
    newSocket->readBlocking(&port, 4);
    int messageSize=0;      
    newSocket->readBlocking(&messageSize, 4);
    char hostname[messageSize+1];
    memset(hostname, 0, messageSize+1);
    newSocket->readBlocking(hostname, messageSize);
    newSocket->setBlocking(false);
    newSocket->setName(string("incoming_from") + hostname);
    BDBSite* newSite = new BDBSite(hostname, port, newSocket);
    
    LOG_TRACE_FLOW_STR("Accepted connection from "<<hostname<<":"<<port);
    itsSiteMap.addSite(newSite);
    return true; // we had something to do
  } else {
    return false;
  }
}

BDBConnector::BDBConnector(const string hostName,
			   const int port,
			   BDBSiteMap& siteMap)
{
  itsRep = new BDBConnectorRep(hostName, port, siteMap);
  itsRep->itsReferences++;
  //  LOG_TRACE_FLOW("BDBConnector constructor");
}

BDBConnector::BDBConnector(const BDBConnector& other)
  : itsRep(other.itsRep)
{
  if (itsRep != 0) 
    itsRep->itsReferences++;
  //  LOG_TRACE_FLOW("BDBConnector copy constructor");
}

BDBConnector::~BDBConnector()
{
  itsRep->itsReferences--;
  if (itsRep->itsReferences == 0)
    delete itsRep;
  //  LOG_TRACE_FLOW("BDBConnector detor");
}

bool BDBConnector::connectTo(string hostName, int port) const
{ return itsRep->connectTo(hostName, port);};
bool BDBConnector::listenOnce()
{
  return itsRep->listenOnce(); 
}
