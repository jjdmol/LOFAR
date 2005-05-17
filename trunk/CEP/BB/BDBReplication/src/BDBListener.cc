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
#include <boost/thread.hpp>

// Application specific includes
#include <BDBListener.h>
#include <BDBConHandler.h>

using namespace LOFAR;
using namespace std;

BDBListenThread::BDBListenThread(const int port,
				 BDBCHThread* ConnectionHandler)
  :itsConnectionHandler(*ConnectionHandler),
   itsPort(port),
   itsShouldStop(false)
{
  //  LOG_TRACE_FLOW("BDBListenThread constructor");
}

BDBListenThread::BDBListenThread(const BDBListenThread& other)
  :itsConnectionHandler(other.itsConnectionHandler),
   itsPort(other.itsPort),
   itsShouldStop(other.itsShouldStop)
{
  //  LOG_TRACE_FLOW("BDBListenThread copy constructor");
}

BDBListenThread::~BDBListenThread()
{
  //  LOG_TRACE_FLOW("BDBListenThread detor");
}

void BDBListenThread::stop()
{
  itsShouldStop = true;
};

bool BDBListenThread::shouldStop()
{
  return itsShouldStop;
};

void BDBListenThread::operator()()
{
  LOG_TRACE_FLOW("BDBListenThread starting thread");
  Socket listenSocket;
  char service[20];
  snprintf(service, 20, "%d", itsPort);
  LOG_TRACE_FLOW_STR("BDBListenThread initting server on port "<<service);
  while (listenSocket.initServer(service) != Socket::SK_OK) {
    LOG_TRACE_FLOW_STR("Could not init server ("<<listenSocket.errstr()<<"), retrying");
  };
  LOG_TRACE_FLOW("Server initted");
  while (!shouldStop()) {
    LOG_TRACE_FLOW("BDBListenThread starting to listen");
    Socket* newSocket = listenSocket.accept();
    if (newSocket != 0){
      int port=0;
      newSocket->readBlocking(&port, 4);
      int messageSize=0;      
      newSocket->readBlocking(&messageSize, 4);
      char hostname[messageSize+1];
      memset(hostname, 0, messageSize+1);
      newSocket->readBlocking(hostname, messageSize);
      BDBSite* newSite = new BDBSite(hostname, port, newSocket);

      LOG_TRACE_FLOW_STR("Accepted connection from "<<newSocket->host()<<":"<<newSocket->port());
      cout<<"Accepted connection from "<<newSocket->host()<<":"<<newSocket->port()<<endl;
      itsConnectionHandler.addSite(newSite);
    }
    LOG_TRACE_FLOW("BDBListenThread accepted connection");
    boost::thread::yield();
  }
}

