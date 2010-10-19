//#  AVTTestMain.cc: Main entry for the Virtual Telescope test
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
#include <Suite/suite.h>
#include "AVTTestTask.h"
#include "AVTTestMAC2Task.h"
#include <boost/shared_ptr.hpp>

using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace AVT;

int main(int argc, char* argv[])
{
  int retval=-1;
 
  INIT_LOGGER(argv[0]);
   
  {
    bool mac1Test=false;
    bool mac2Test=false;
    
    GCFTask::init(argc, argv);

    CCmdLine cmdLine;

    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      AVTTestTask::m_sBeamServerOnly=cmdLine.HasSwitch("-b");
      mac1Test = cmdLine.HasSwitch("-mac1");
      mac2Test = cmdLine.HasSwitch("-mac2");
    }
    
    if(!mac1Test && !mac2Test)
    {
      printf("Please use the commandline parameters '-mac1' or '-mac2' or both to\nexecute MAC1 or MAC2 testcases\n");
    }
    else
    {
      Suite s("MAC.APL.PAC VirtualTelescope Test",&std::cout);
    
      boost::shared_ptr<AVTTestTask>      avtTestMac1;
      boost::shared_ptr<AVTTestMAC2Task>  avtTestMac2;
      if(mac1Test)
      {
        avtTestMac1 = boost::shared_ptr<AVTTestTask>(new AVTTestTask);
        s.addTest(avtTestMac1.get());
      }
      if(mac2Test)
      {
        avtTestMac2 = boost::shared_ptr<AVTTestMAC2Task>(new AVTTestMAC2Task);
        s.addTest(avtTestMac2.get());
      }
      s.run();
      retval=s.report();
      s.free();
    }
  }
  return retval;
}

