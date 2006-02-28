//#  tProcess.cc: one line description
//#
//#  Copyright (C) 2005
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/Process.h>
#include <Common/LofarLogger.h>
#include <cerrno>
#include <unistd.h>    // for getpid() and sleep()

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


int main (int /*argc*/, const char* argv[])
{
  INIT_LOGGER (argv[0]);

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
  if (system("ps -C tProcess --no-header -ostate") == -1) {
    LOG_FATAL_STR("system() command failed: " << strerror(errno));
    return 1;
  }

  return 2;  // forces assay to flag missing .stdout file as an error
}
