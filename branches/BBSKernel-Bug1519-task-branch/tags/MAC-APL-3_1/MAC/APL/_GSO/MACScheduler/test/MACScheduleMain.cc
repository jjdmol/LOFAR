//#  MACScheduleMain.cc: Main entry for the MAC commandline scheduler
//#
//#  Copyright (C) 2002-2004
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

#include <Common/LofarLogger.h>
#include <GCF/CmdLine.h>
#include <GCF/TM/GCF_Task.h>
#include "MACScheduleTask.h"
#include <boost/shared_ptr.hpp>

using namespace LOFAR;
using namespace GSO;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

void usage()
{
  printf("usage: MACSchedule (-s|-u|-c) <nodeId>\n");
  printf("  -s = schedule, -u = update schedule -c = cancel schedule\n");
}

int main(int argc, char* argv[])
{
  int retval=-1;
 
  {
    GCFTask::init(argc, argv);

    CCmdLine cmdLine;
    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      if(cmdLine.HasSwitch("-s") || cmdLine.HasSwitch("-u") || cmdLine.HasSwitch("-c"))
      {
        MACScheduleTask ms(cmdLine.GetArgument("-s",0),cmdLine.GetArgument("-u",0),cmdLine.GetArgument("-c",0));
        ms.start(); // make initial transition
        GCFTask::run();
      }
      else
      {
        usage();
      }
    }
    else
    {
      usage();
    }
  }
  return retval;
}

