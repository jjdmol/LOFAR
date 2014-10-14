//#  THPVSSBridgeMain.cc: Main entry for the THPVSSBridge
//#
//#  Copyright (C) 2002-2005
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
#include "THPVSSBridge.h"

using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::MACTest;

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);
  
  string id("1");
  if (argv != 0)
  {
    CCmdLine cmdLine;

    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      id = cmdLine.GetSafeArgument("-id", 0, "1");
    }            
  }
  string taskName = string("THPVSSBRIDGE") + id;
  LOG_INFO(formatString("Starting task %s. Use %s -id <id> to create a task with a different id",taskName.c_str(),argv[0],argv[0]));

  THPVSSBridge pvssBridge(taskName);
  pvssBridge.start(); // make initial transition

  GCFTask::run();
    
  return 0;
}

