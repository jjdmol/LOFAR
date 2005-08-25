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
#include <Transport/TransportHolder.h>
#include <Transport/TH_Socket.h>

// Application specific includes
#include <BDBConnector.h>


namespace LOFAR{
  namespace BDBReplication {
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
  TransportHolder* th = new TH_Socket(newSocket);
  BDBMessage message(BDBMessage::CONNECT);
  message.setHostName(hostName);
  message.setPort(port);
  BDBSite* newSite = new BDBSite(hostName.c_str(), port, th);

  if (message.send(th)) {
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
    BDBMessage message;
    TransportHolder* th = new TH_Socket(newSocket);
    message.receive(th);
    cerr<<"connect message received"<<endl;
    LOG_TRACE_FLOW_STR("Accepted connection from "<<message.getHostName()<<":"<<message.getPort());
    DBGASSERTSTR(message.getType() == BDBMessage::CONNECT, "Connector received wrong packet");
    newSocket->setName(string("incoming_from") + message.getHostName());
    BDBSite* newSite = new BDBSite(message.getHostName().c_str(), message.getPort(), th);
    
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
}
}
