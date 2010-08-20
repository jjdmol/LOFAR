//# tSystemUtil.cc: test program for the SystemUtil source
//#
//# Copyright (C) 2006-2007
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

#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace LOFAR;

void testHostname()
{
  string hostname;
  hostname = myHostname(false);
  ASSERT(!hostname.empty());
  cout << "My short hostname is: " << hostname << endl;
  hostname = myHostname(true);
  ASSERT(!hostname.empty());
  cout << "My long hostname is : " << hostname << endl;
}

void testIP()
{
  uint32 address = myIPV4Address();
  ASSERT(address != 0);
  cout << formatString("My IPV4 address is  : %08lX\n", ntohl(address));
  hexdump ((char*) &address, sizeof(uint32));
}

void testGetExePath()
{
  string exePath = getExecutablePath();
  ASSERT(!exePath.empty());
  cout << "My executable path  : " << getExecutablePath() << endl;
}

void testDirnameBasename()
{
  const char* paths[] = { 
    "/usr/lib", "/usr/", "usr", "/", "///", 
    "//usr//lib", ".", "..", "../a", "../a/.b"
  };
  cout << "Test of getDirname() and getBasename() :" << endl;
  for(int i = 0; i < 10; i++) {
    cout << setw(11) << left << paths[i] << "--> ('" 
         << getDirname(paths[i]) << "', '" 
         << getBasename(paths[i]) << "')" << endl;
  }
}

int main()
{
  INIT_LOGGER("tSystemUtil");
  try {
    testHostname();
    testIP();
    testGetExePath();
    testDirnameBasename();
  } catch(Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return (0);
}
