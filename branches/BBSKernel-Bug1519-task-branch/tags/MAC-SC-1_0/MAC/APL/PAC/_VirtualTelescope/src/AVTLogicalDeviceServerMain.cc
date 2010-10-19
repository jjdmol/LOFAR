//#  AVTLogicalDeviceServerMain.cc: Main entry for the Logical Device Server
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
#include "AVTLogicalDeviceScheduler.h"
#include <GCF/ParameterSet.h>

using namespace LOFAR;
using namespace LOFAR::AVT;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;

int main(int argc, char* argv[])
{
  INIT_LOGGER(argv[0]);
  LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"LogicalDeviceScheduler");
  
  GCFTask::init(argc, argv);
  
  ParameterSet::instance()->adoptFile("LogicalDeviceServer.conf");
    
  AVTLogicalDeviceScheduler lds;
  lds.start(); // make initial transition

  GCFTask::run();
    
  return 0;
}

