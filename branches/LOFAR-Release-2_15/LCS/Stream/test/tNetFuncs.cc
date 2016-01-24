//# tNetFuncs.cc
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
#include "../src/NetFuncs.h"

#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace std;

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void test_getInterfaceIP()
{
  // Loopback device (lo) should have IP 127.0.0.1
  struct sockaddr sa = getInterfaceIP("lo");

  struct sockaddr_in *sai = (struct sockaddr_in*)&sa;
  const string ip = inet_ntoa(sai->sin_addr);
  ASSERTSTR(ip == "127.0.0.1", "Loopback device reports IP " << ip);
}

void test_safeGetAddrInfo()
{
  // "localhost" should also resolve to 127.0.0.1
  safeAddrInfo addr;
  safeGetAddrInfo(addr, true, "localhost", 0);

  struct sockaddr_in *sai = (struct sockaddr_in*)addr.addrinfo->ai_addr;
  const string ip = inet_ntoa(sai->sin_addr);
  ASSERTSTR(ip == "127.0.0.1", "localhost resolves to " << ip);
}

void test()
{
  test_getInterfaceIP();
  test_safeGetAddrInfo();
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
