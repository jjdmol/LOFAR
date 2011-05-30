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

#define CHECK(cond)                      \
  do {                                   \
    if (!(cond)) {                       \
      errors++;                          \
      LOG_ERROR("Check failed: " #cond); \
    }                                    \
  } while(0)

#define CHECK_STR(cond, strm)                             \
  do {                                                    \
    if(!(cond))  {                                        \
      errors++;                                           \
      LOG_ERROR_STR("Check failed: " #cond "; " << strm); \
    } else {                                              \
      LOG_INFO_STR(strm);                                 \
    }                                                     \
  } while(0)

using namespace LOFAR;

int errors;

void testHostname()
{
  string hostname;
  hostname = myHostname(false);
  CHECK(!hostname.empty());
  LOG_INFO_STR("My short hostname is: " << hostname);
  hostname = myHostname(true);
  CHECK(!hostname.empty());
  LOG_INFO_STR("My long hostname is : " << hostname);
}

void testIP()
{
  uint32 address = myIPV4Address();
  string dump;
  CHECK_STR(address != 0, 
            formatString("My IPV4 address is  : %08lX", ntohl(address)));
  hexdump (dump, (char*) &address, sizeof(address));
  LOG_INFO(dump);
}

void testGetExePath()
{
  string exePath = getExecutablePath();
  CHECK(!exePath.empty());
  LOG_INFO_STR("My executable path  : " << getExecutablePath());
}

void testDirnameBasename()
{
  struct test {
    const char* path;
    const char* suffix;
    const char* dir;
    const char* base;
  };
  test tests [] = {
    //    path        suffix   dirname   basename
    { "/usr/lib"    ,  ""    ,  "/usr"  ,  "lib" },
    { "/usr/"       ,  ""    ,  "/"     ,  "usr" },
    { "usr"         ,  ""    ,  "."     ,  "usr" },
    { "/"           ,  ""    ,  "/"     ,  "/"   },
    { "."           ,  ""    ,  "."     ,  "."   },
    { ".."          ,  ""    ,  "."     ,  ".."  },
    { "///"         ,  ""    ,  "/"     ,  "/"   },
    { "//"          ,  ""    ,  "/"     ,  "/"   },
    { "//usr//lib//",  ""    ,  "//usr" ,  "lib" },
    { "//usr//lib"  ,  ""    ,  "//usr" ,  "lib" },  
    { "../foo"      ,  ""    ,  ".."    ,  "foo" }, 
    { "foo/bar"     ,  "bar" ,  "foo"   ,  "bar" }, 
    { "foo/bar.baz" ,  ".baz",  "foo"   ,  "bar" }
  };
  for(size_t i = 0; i < sizeof(tests)/sizeof(test); i++) {
    test& t = tests[i];
    CHECK_STR(dirname(t.path) == t.dir,
              "dirname(\"" << t.path << "\") == " << t.dir);
    CHECK_STR(basename(t.path, t.suffix) == t.base,
              "basename(\"" << t.path << "\", \"" << t.suffix << 
              "\") == " << t.base);
//     cout << "echo \"dirname(\\\"" << t.path 
//          << "\\\") == `dirname " << t.path << "`\"" << endl;
//     cout << "echo \"basename(\\\"" << t.path << "\\\", \\\"" << t.suffix 
//          << "\\\")" << " == `basename " << t.path << " " << t.suffix << "`\""
//          << endl;
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
