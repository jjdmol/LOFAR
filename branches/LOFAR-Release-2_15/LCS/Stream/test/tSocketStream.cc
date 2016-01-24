//# tSocketStream.cc
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
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Stream/SocketStream.h>
#include <Common/LofarLogger.h>
#include <Common/Thread/Thread.h>

using namespace LOFAR;
using namespace std;

class Server {
public:
  Server(SocketStream::Protocol protocol)
  :
    protocol(protocol),
    ss("localhost", 0, protocol, SocketStream::Server, 0, false),
    thread(this, &Server::mainLoop)
  {
    LOG_INFO_STR("Server listening on port " << ss.getPort());
  }

  ~Server()
  {
    thread.cancel();
  }

  void mainLoop() {
    if (protocol == SocketStream::TCP)
      ss.reaccept();
  }

  int getPort() const { return ss.getPort(); }

private:
  SocketStream::Protocol protocol;
  SocketStream ss;
  Thread thread;
};

void test_client_server(SocketStream::Protocol protocol)
{
  // Waits for a connection in the background
  Server s(protocol);

  // Setup clients, bound to loopi-back interface
  SocketStream c("localhost", s.getPort(), protocol, SocketStream::Client, 0, false);
}

void test()
{
  LOG_INFO("Testing TCP...");
  test_client_server(SocketStream::TCP);

  LOG_INFO("Testing UDP...");
  test_client_server(SocketStream::UDP);
}


int main(int /*argc*/, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {
    alarm(30);

    test();

  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  LOG_INFO("Program terminated successfully");
  return 0;
}
