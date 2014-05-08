//# logperf.cc
//#
//# Copyright (C) 2002-2004
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
#include <sys/time.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace LOFAR;

const unsigned max_run = 100000;

// Run function <func> and return the time it took to run it in seconds.
double time(void (*func)())
{
  static timeval start, stop;
  if(gettimeofday(&start, 0) != 0) abort();
  func();
  if(gettimeofday(&stop, 0) != 0) abort();
  return (stop.tv_sec - start.tv_sec) + 1e-6*(stop.tv_usec - start.tv_usec);
}

void run0()
{
  for(unsigned i = 0; i < max_run; i++) {
#ifdef HAVE_LOG4CPLUS
    log4cplus::Logger::getInstance("LCS.Common").isEnabledFor(log4cplus::INFO_LOG_LEVEL);
#elif HAVE_LOG4CXX
    log4cxx::Logger::getLogger("LCS.Common")->isInfoEnabled();
#endif
  }
}

void run1()
{
  for(unsigned i = 0; i < max_run; i++) {
    LOG_INFO("Hello world");
  }
}

void run2a()
{
  for(unsigned i = 0; i < max_run; i++) {
    LOG_INFO(formatString("%s %s", "Hello", "World"));
  }
}

void run2b()
{
  for(unsigned i = 0; i < max_run; i++) {
    LOG_INFO_STR("Hello " << "World");
  }
}

void run3a()
{
  for(unsigned i = 0; i < max_run; i++) {
    LOG_INFO(formatString("This is a double: %1.14f", 2.718281828459045));
  }
}

void run3b()
{
  for(unsigned i = 0; i < max_run; i++) {
    LOG_INFO_STR("This is a double: " << setprecision(15) << 2.718281828459045);
  }
}

int main()
{
  INIT_LOGGER("logperf");
  clog << "run 0  took: " << time(run0)  << " seconds" << endl;
  clog << "run 1  took: " << time(run1)  << " seconds" << endl;
  clog << "run 2a took: " << time(run2a) << " seconds" << endl;
  clog << "run 2b took: " << time(run2b) << " seconds" << endl;
  clog << "run 3a took: " << time(run3a) << " seconds" << endl;
  clog << "run 3b took: " << time(run3b) << " seconds" << endl;
}
