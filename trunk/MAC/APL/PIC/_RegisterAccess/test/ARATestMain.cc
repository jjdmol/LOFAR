//#  ARATestMain.cc: Main entry for the Register Access test
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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#undef PACKAGE
#undef VERSION
#include <GCF/CmdLine.h>
#include <GCF/TM/GCF_Task.h>
#include "ARATestTask.h"
#include "ARATestDriverTask.h"
#include <boost/shared_ptr.hpp>
#include <Suite/suite.h>

using namespace std;

using namespace LOFAR;
using namespace ARA;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
  
int main(int argc, char* argv[])
{
  int retval=-1;
  
  {
    GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));
  
    CCmdLine cmdLine;

    bool noTest=false;
    bool schedule=false;
    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      noTest = cmdLine.HasSwitch("-notest");
      schedule = cmdLine.HasSwitch("-schedule");
    }
    
    // create test driver task. 
    if(noTest)
    {
      ARATestDriverTask testDriverTask;
      testDriverTask.start(); // make initial transition
      GCFTask::run(); //is also called by the ARATest class
    }
    else if(schedule)
    {
      ARATestDriverTask testDriverTask;
      testDriverTask.schedule(cmdLine.GetSafeArgument("-schedule", 0, "VI1.ps"));
      testDriverTask.start(); // make initial transition
      GCFTask::run(); //is also called by the ARATest class
    }
    else
    {
      Suite s("MAC.APL.PIC RegisterAccess Test",&std::cout);
    
      boost::shared_ptr<ARATestTask> araTest(new ARATestTask);
      s.addTest(araTest.get());
      s.run();
      retval=s.report();
      s.free();
    }
  }
  return retval;
}


