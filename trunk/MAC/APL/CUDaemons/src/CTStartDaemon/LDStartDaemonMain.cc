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
//#undef PACKAGE
//#undef VERSION
#include <lofar_config.h>
#include <signal.h>
#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>
#include <APL/STSCtl/StartDaemon.h>
//#include "VirtualInstrument.h"
//#include "MaintenanceVI.h"
//#include "ObservationVI.h"
//#include <APL/ArrayReceptorGroup/ArrayReceptorGroup.h>
//#include <APL/ArrayOperations/ArrayOperations.h>
//#include <APL/VirtualRoute/VirtualRoute.h>
#ifdef CREATING_TASKS
#include <APL/STSCtl/LogicalDeviceFactory.h>
#include "LDtest.h"
#else
#include <APL/STSCtl/LDstarter.h>
#endif
//#include <APL/STSCtl/SharedLogicalDeviceFactory.h>

using namespace LOFAR;
using namespace LOFAR::STSCtl;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::APLCommon;
//using namespace LOFAR::AVI;  // A)pplication layer V)irtual I)nstrument
//using namespace LOFAR::AAO;  // A)pplication layer A)rray O)perations
//using namespace LOFAR::AAR;  // A)pplication layer A)rray R)eceptor group
//using namespace LOFAR::AVR;  // A)pplication layer V)irtual R)oute

int main(int argc, char* argv[])
{
	signal (SIGCHLD, SIG_IGN);

	GCFTask::init(argc, argv);
  
#ifdef CREATING_TASKS
	boost::shared_ptr<LogicalDeviceFactory<LDtest> >	testFactory(new LogicalDeviceFactory<LDtest>);
#else
	boost::shared_ptr<LDstarter>        				testFactory(new LDstarter("./LDtestMain"));
#endif

//  boost::shared_ptr<LogicalDeviceFactory<VirtualInstrument> >        viFactory(new LogicalDeviceFactory<VirtualInstrument>);
//  boost::shared_ptr<SharedLogicalDeviceFactory<ArrayReceptorGroup> > argFactory(new SharedLogicalDeviceFactory<ArrayReceptorGroup>);
//  boost::shared_ptr<SharedLogicalDeviceFactory<ArrayOperations> >    aoFactory(new SharedLogicalDeviceFactory<ArrayOperations>);
//  boost::shared_ptr<LogicalDeviceFactory<VirtualRoute> >             vrFactory(new LogicalDeviceFactory<VirtualRoute>);
//  boost::shared_ptr<LogicalDeviceFactory<MaintenanceVI> >            mviFactory(new LogicalDeviceFactory<MaintenanceVI>);
//  boost::shared_ptr<LogicalDeviceFactory<ObservationVI> >            oviFactory(new LogicalDeviceFactory<ObservationVI>);
  
	StartDaemon sd(string("VIC_VIStartDaemon"));		// give myself a name

	sd.registerFactory(LDTYPE_VIRTUALINSTRUMENT,testFactory);

//  sd.registerFactory(LDTYPE_VIRTUALINSTRUMENT,viFactory);
//  sd.registerFactory(LDTYPE_ARRAYRECEPTORGROUP,argFactory);
//  sd.registerFactory(LDTYPE_ARRAYOPERATIONS,aoFactory);
//  sd.registerFactory(LDTYPE_VIRTUALROUTE,vrFactory);
//  sd.registerFactory(LDTYPE_MAINTENANCEVI,mviFactory);
//  sd.registerFactory(LDTYPE_OBSERVATION,oviFactory);

	sd.start(); // make initial transition

	GCFTask::run();
    
	return 0;
}

