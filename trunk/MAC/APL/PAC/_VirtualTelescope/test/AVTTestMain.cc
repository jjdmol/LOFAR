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

#include <CmdLine.h>
#include <GCF/GCF_Task.h>
#include "../../../APLCommon/src/suite.h"
#include "AVTTest.h"
#include <boost/shared_ptr.hpp>

int main(int argc, char* argv[])
{
  int retval=-1;
  
  {
    GCFTask::init(argc, argv);

    CCmdLine cmdLine;

    // parse argc,argv 
    if (cmdLine.SplitLine(argc, argv) > 0)
    {
      AVTTestTask::m_sBeamServerOnly=cmdLine.HasSwitch("-b");
    }
    
    Suite s("MAC.APL.PAC VirtualTelescope Test",&std::cout);
  
    boost::shared_ptr<AVTTest> avtTest(new AVTTest);
    s.addTest(avtTest.get());
    s.run();
    retval=s.report();
  }
  return retval;
}

