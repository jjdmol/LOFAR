//# PortBroker.cc: 
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: PortBroker.cc 20465 2012-03-16 15:53:48Z mol $

#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <Common/Thread/Cancellation.h>
#include <Stream/PortBroker.h>

#include <cstdio>
#include <time.h>

#ifdef USE_THREADS

using namespace std;

namespace LOFAR {

static auto_ptr<PortBroker> pbInstance;

void PortBroker::createInstance( uint16 port )
{
  ASSERTSTR(!pbInstance.get(), "PortBroker instance already created");

  pbInstance.reset(new PortBroker(port));

  instance().start();
}

PortBroker &PortBroker::instance()
{
  return *pbInstance;
}


PortBroker::PortBroker( uint16 port )
:
  SocketStream( "0.0.0.0", port, TCP, Server, 0, "", false ),
  itsDone(false)
{
}

PortBroker::~PortBroker()
{
  ASSERT( itsThread.get() );

  // break serverLoop explicitly
  itsThread->cancel();

  {
    ScopedLock sl(itsMutex);

    // release all unfulfilled requests
    for( requestMapType::iterator it = itsRequestMap.begin(); it != itsRequestMap.end(); ++it ) {
      LOG_DEBUG_STR( "PortBroker request: discarding " << it->first );
      delete it->second;
    }

    // break any waitForClient conditional waits
    itsDone = true;
    itsCondition.broadcast();
  }  
}


void PortBroker::start() {
  itsThread.reset(new Thread(this, &PortBroker::serverLoop, "[PortBroker] ", 65535));
}


size_t PortBroker::nrOutstandingRequests() const {
  ScopedLock sl(itsMutex);

  return itsRequestMap.size();
}


void PortBroker::PortRequest::set( const string &_resource )
{
  ASSERTSTR( _resource.size() < sizeof resource, "Requested resource '" << _resource << "' is longer than the name limit (" << sizeof resource << ")");

  snprintf( resource, sizeof resource, "%s", _resource.c_str() );
}


string PortBroker::PortRequest::key() const
{
  return string(resource);
}


void PortBroker::PortRequest::read( Stream &stream )
{
  // send request
  stream.read( &resource, sizeof resource );

  // fix mal-formed requests
  resource[sizeof resource - 1] = 0;

  LOG_DEBUG_STR( "PortBroker request: received " << key() );
}


void PortBroker::PortRequest::write( Stream &stream )
{
  LOG_DEBUG_STR( "PortBroker request: sending " << key() );

  // send request
  stream.write( &resource, sizeof resource );
}


void PortBroker::serverLoop()
{
  while(!itsDone) {
    // wait for new client to arrive
    reaccept();

    // disown the file descriptor
    auto_ptr<FileDescriptorBasedStream> clientStream(detach());

    // obtain the request
    PortRequest request;

    request.read(*clientStream);
    const string &key = request.key();

    // put the request in our map
    {
      ScopedLock sl(itsMutex);

      ASSERTSTR( itsRequestMap.find(key) == itsRequestMap.end(), "Multiple requests pending for key " << key );

      itsRequestMap[key] = clientStream.release();

      // signal a change in itsRequestMap
      itsCondition.broadcast();
    }
  }
}


bool PortBroker::serverStarted()
{
  return pbInstance.get() != NULL && pbInstance->itsThread.get() != NULL;
}


FileDescriptorBasedStream *PortBroker::waitForClient( const string &resource, time_t deadline ) {
  struct timespec deadline_ts = { deadline, 0 };

  LOG_DEBUG_STR( "PortBroker server: registering " << resource );

  PortRequest request;

  request.set(resource);
  const string &key = request.key();

  ScopedLock sl(itsMutex);

  while(!itsDone) {
    requestMapType::iterator it = itsRequestMap.find(key);

    if (it != itsRequestMap.end()) {
      auto_ptr<FileDescriptorBasedStream> serverStream(it->second);

      itsRequestMap.erase(it);

      return serverStream.release();
    }

    if (deadline > 0) {
      if (!itsCondition.wait(itsMutex, deadline_ts))
        THROW(TimeOutException, "port broker client: server did not register");
    } else {
      itsCondition.wait(itsMutex);
    }
  }

  return 0;
}


void PortBroker::requestResource(Stream &stream, const string &resource)
{
  PortRequest request;

  request.set(resource);
  request.write(stream);
}


PortBroker::ServerStream::ServerStream( const string &resource )
{
  ASSERTSTR( serverStarted(), "PortBroker service is not started" );

  // wait for client to request our service
  auto_ptr<FileDescriptorBasedStream> stream(PortBroker::instance().waitForClient(resource));

  // transfer ownership
  fd = stream->fd;
  stream->fd = -1;
}


PortBroker::ClientStream::ClientStream( const string &hostname, uint16 port, const string &resource, time_t deadline )
:
  // connect to port broker
  SocketStream(hostname, port, SocketStream::TCP, SocketStream::Client, deadline)
{
  // request service
  PortBroker::requestResource(*this, resource);
}


} // namespace LOFAR

#endif

