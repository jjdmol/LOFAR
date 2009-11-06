//# tProcess.cc: one line description
//#
//# Copyright (C) 2005
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
#include <Common/Process.h>
#include <Common/LofarLogger.h>
#include <cerrno>
#include <cstdlib>     // for exit() and system()
#include <cstring>     // for strerror()
#include <unistd.h>    // for getpid() and sleep()
#include <libgen.h>    // for basename()

using namespace std;
using namespace LOFAR;


class MyProcess : public Process
{
  virtual void parent()
  {
    LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME << ": pid = " << getpid());
  }

  virtual void child() 
  {
    pid_t pid = getpid();
    pid_t ppid = getppid();
    LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME << ": pid = " << pid
                       << "; ppid = " << ppid);
    LOG_TRACE_STAT_STR(AUTO_FUNCTION_NAME
                       << ": going to sleep for 2 seconds ...");
    sleep(2);
    LOG_TRACE_STAT_STR(AUTO_FUNCTION_NAME
                       << ": pid #" << pid << " exiting ...");
    exit(0); 
  }
};


int main (int /*argc*/, char* argv[])
{
  string prog(basename(argv[0]));
  INIT_LOGGER (prog.c_str());
  
  try {
    MyProcess proc;
    LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME 
                       << ": create a zombie process ...");
    proc.spawn();

    LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME 
                       << ": sleeping for 1 second ...");
    sleep(1);

    LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME 
                       << ": create a non-zombie process ...");
    proc.spawn(true);

  }
  catch (Exception& e) {
    LOG_FATAL_STR(e);
    return 1;
  }

  LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME 
                     << ": server is going to sleep for 4 seconds now ...");
  sleep(4);

  LOG_TRACE_FLOW_STR(AUTO_FUNCTION_NAME 
                     << ": server woke up"
                     << ", checking state of child processes ...");
  string cmd = "ps -C " + prog + " --no-header -ostate -ocomm -ostate | grep '"
               + prog + " ' | sed -e 's/ tProcess.*//'" ;
  if (system(cmd.c_str()) == -1) {
    LOG_FATAL_STR("system() command failed: " << strerror(errno));
    return 1;
  }

  return 0;
}
