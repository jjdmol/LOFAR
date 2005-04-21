//#  VBMain.cc: Main entry for the VirtualBackend start daemon
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
#include "VirtualBackendFactory.h"

using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::APLCommon;
using namespace LOFAR::AVB;  // A)pplication layer V)irtual B)ackend

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);

  LOG_INFO("MACProcessScope: APL.PAC.VB");  
  
  boost::shared_ptr<VirtualBackendFactory> vbFactory(new VirtualBackendFactory);
  
  StartDaemon sd(string("CEPLCU_PAC_VBStartDaemon"));
  sd.registerFactory(LDTYPE_VIRTUALBACKEND, vbFactory);
  sd.start(); // make initial transition

  GCFTask::run();
    
  return 0;
}

