//#  VirtualInstrumentStartDaemonMain.cc: Main entry for the VirtualInstrument start daemon
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

#include <boost/shared_ptr.hpp>

#include <APLCommon/StartDaemon.h>
#include "VirtualInstrumentFactory.h"

using namespace LOFAR;
using namespace APLCommon;
using namespace AVI;  // A)pplication layer V)irtual I)nstrument

int main(int argc, char* argv[])
{
  INIT_LOGGER(argv[0]);

  GCFTask::init(argc, argv);
  
  string ccuName("CCU1");
  if(argc != 2)
  {
    LOG_FATAL(formatString("invalid number of arguments. Assuming: %s %s",argv[0],ccuName.c_str()));
  }
  else
  {
    ccuName = string(argv[1]);
    LOG_INFO(formatString("VirtualInstrument started with CCU name = %s",ccuName.c_str()));
  }
  
  boost::shared_ptr<VirtualInstrumentFactory> viFactory(new VirtualInstrumentFactory);
  
  StartDaemon sd(ccuName + string("_VIC_VIStartDaemon"));
  sd.registerFactory(LDTYPE_VIRTUALINSTRUMENT,viFactory);
  sd.start(); // make initial transition

  GCFTask::run();
    
  return 0;
}

