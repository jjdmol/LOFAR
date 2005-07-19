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

BDBConnectorRep::BDBConnectorRep(const int port,
				 BDBCommunicator* ConnectionHandler)
  :itsConnectionHandler(*ConnectionHandler),
   itsPort(port),
   itsShouldStop(false),
   itsIsListening(false)
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

void BDBConnectorRep::operator()()
{
  LOG_TRACE_FLOW("BDBConnector starting thread");
  Socket listenSocket;
  char service[20];
  snprintf(service, 20, "%d", itsPort);
  LOG_TRACE_FLOW_STR("BDBConnector initting server on port "<<service);
  while (!itsIsListening) {    
    char service[20];
    snprintf(service, 20, "%d", itsPort);
    if (listenSocket.initServer(service) != Socket::SK_OK) {
      LOG_TRACE_FLOW_STR("Could not init server ("<<listenSocket.errstr()<<"), retrying");
      itsPort++;
    } else {
      itsIsListening = true;
    }
  };
  LOG_TRACE_FLOW("BDBConnector starting to listen");
  while (!shouldStop()) {
    Socket* newSocket = listenSocket.accept(500); // wait 500 ms for new connection
    if (newSocket != 0){
      int port=0;
      newSocket->readBlocking(&port, 4);
      int messageSize=0;      
      newSocket->readBlocking(&messageSize, 4);
      char hostname[messageSize+1];
      memset(hostname, 0, messageSize+1);
      newSocket->readBlocking(hostname, messageSize);
      BDBSite* newSite = new BDBSite(hostname, port, newSocket);

      LOG_TRACE_FLOW_STR("Accepted connection from "<<hostname<<":"<<port);
      itsConnectionHandler.addSite(newSite);
    }
    boost::thread::yield();
  }
}

BDBConnector::BDBConnector(const int port,
			   BDBCommunicator* ConnectionHandler)
{
  itsRep = new BDBConnectorRep(port, ConnectionHandler);
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
