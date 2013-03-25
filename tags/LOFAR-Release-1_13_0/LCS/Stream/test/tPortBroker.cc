//# tAllocator.cc: test program for the Allocator class
//#
//# Copyright (C) 2006
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
//# $Id: tAllocator.cc 14057 2009-09-18 12:26:29Z diepen $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Stream/PortBroker.h>
#include <Stream/SocketStream.h>
#include <Common/Thread/Thread.h>
#include <Common/LofarLogger.h>
#include <string>
#include <unistd.h>

#ifdef USE_THREADS

using namespace LOFAR;
using namespace std;

uint16 first_port = 4000;
uint16 last_port  = 5000;
uint16 PORT;

void start()
{
  ASSERT( first_port < last_port );

  for (PORT = first_port; PORT < last_port; ++PORT) {
    try {
      PortBroker::createInstance(PORT);

      // found a valid port
      LOG_DEBUG_STR( "using TCP port " << PORT );
      break;
    } catch( SocketStream::BindException& ) {
      // port is in use -- try the next one
      continue;
    }
  }

  if (PORT >= last_port)
    ASSERTSTR( false, "Could not find a free TCP port between " << first_port << " and " << last_port );
}

class OneRequest
{
  public:
    OneRequest( const string &resource, unsigned msg ): resource(resource), msg(msg) {}

    void clientThread();
    void serverThread();
  private:
    const string resource;
    const unsigned msg;
};

void OneRequest::clientThread() {
  PortBroker::ClientStream cs("127.0.0.1", PORT, resource);

  unsigned x;

  cs.read( &x, sizeof x );

  ASSERT(x == msg);
}

void OneRequest::serverThread() {
  PortBroker::ServerStream ss(resource);

  unsigned x = msg;

  ss.write( &x, sizeof x );
}

void one_request()
{
  OneRequest obj("test", 42);

  LOG_DEBUG( "one_request: client first" );

  {
    // client first
    Thread ct(&obj, &OneRequest::clientThread);
    sleep(1);

    Thread st(&obj, &OneRequest::serverThread);
  }  

  ASSERT( PortBroker::instance().nrOutstandingRequests() == 0 );

  LOG_DEBUG( "one_request: server first" );

  {
    // server first
    Thread st(&obj, &OneRequest::serverThread);
    sleep(1);

    Thread ct(&obj, &OneRequest::clientThread);
  }  

  ASSERT( PortBroker::instance().nrOutstandingRequests() == 0 );
}

void two_requests()
{
  OneRequest obj1("foo", 12345);
  OneRequest obj2("bar", 67890);

  LOG_DEBUG( "two_requests: clients first" );

  {
    // client first
    Thread ct1(&obj1, &OneRequest::clientThread);
    Thread ct2(&obj2, &OneRequest::clientThread);
    sleep(1);

    Thread st1(&obj1, &OneRequest::serverThread);
    Thread st2(&obj2, &OneRequest::serverThread);
  }  

  ASSERT( PortBroker::instance().nrOutstandingRequests() == 0 );

  LOG_DEBUG( "two_requests: servers first" );

  {
    // client first
    Thread st1(&obj1, &OneRequest::serverThread);
    Thread st2(&obj2, &OneRequest::serverThread);
    sleep(1);

    Thread ct1(&obj1, &OneRequest::clientThread);
    Thread ct2(&obj2, &OneRequest::clientThread);
  }

  ASSERT( PortBroker::instance().nrOutstandingRequests() == 0 );
}

int main(int /*argc*/, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  try {
    alarm(30);

    start();

    one_request();
    two_requests();
  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  LOG_INFO("Program terminated successfully");
  return 0;
}

#else // USE_THREADS

int main(int /*argc*/, const char* /*argv[]*/)
{
  return 0;
}

#endif
