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
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>
#include <APLCommon/StartDaemon.h>
#include "VirtualInstrumentFactory.h"
#include "MaintenanceVIFactory.h"
#include "ObservationVIFactory.h"
#include <ArrayReceptorGroup/ArrayReceptorGroupFactory.h>
#include <ArrayOperations/ArrayOperationsFactory.h>

using namespace LOFAR;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF::PAL;
using namespace LOFAR::APLCommon;
using namespace LOFAR::AVI;  // A)pplication layer V)irtual I)nstrument
using namespace LOFAR::AAO;  // A)pplication layer A)rray O)perations
using namespace LOFAR::AAR;  // A)pplication layer A)rray R)eceptor group

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);
  
  boost::shared_ptr<VirtualInstrumentFactory>   viFactory(new VirtualInstrumentFactory);
  boost::shared_ptr<ArrayReceptorGroupFactory>  argFactory(new ArrayReceptorGroupFactory);
  boost::shared_ptr<ArrayOperationsFactory>     aoFactory(new ArrayOperationsFactory);
  boost::shared_ptr<MaintenanceVIFactory>       mviFactory(new MaintenanceVIFactory);
  boost::shared_ptr<ObservationVIFactory>       oviFactory(new ObservationVIFactory);
  
  StartDaemon sd(string("VIC_VIStartDaemon"));
  sd.registerFactory(LDTYPE_VIRTUALINSTRUMENT,viFactory);
  sd.registerFactory(LDTYPE_ARRAYRECEPTORGROUP,argFactory);
  sd.registerFactory(LDTYPE_ARRAYOPERATIONS,aoFactory);
  sd.registerFactory(LDTYPE_MAINTENANCEVI,mviFactory);
  sd.registerFactory(LDTYPE_OBSERVATION,oviFactory);
  sd.start(); // make initial transition

  GCFTask::run();
    
  return 0;
}

