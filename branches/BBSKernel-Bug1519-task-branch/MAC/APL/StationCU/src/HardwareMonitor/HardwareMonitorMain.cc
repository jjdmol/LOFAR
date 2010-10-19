//#  HardwareMonitor.cc: Main entry for the HardwareMonitor.
//#
//#  Copyright (C) 2006
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
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>

#include <Common/ParameterSet.h>
#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <GCF/RTDB/DP_Protocol.ph>
#include "RSPMonitor.h"
#include "TBBMonitor.h"
#include "ECMonitor.h"
#include <StationCU/Package__Version.h>

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::StationCU;

int main(int argc, char* argv[])
{
	// args: cntlrname, parentHost, parentService
	GCFScheduler::instance()->init(argc, argv, "HardwareMonitor");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_HardwareMonitor");
	LOG_INFO(Version::getInfo<StationCUVersion>("HardwareMonitor"));

	// for debugging purposes
	registerProtocol (RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);
	registerProtocol (TBB_PROTOCOL, TBB_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL,  DP_PROTOCOL_STRINGS);

	// Create tasks and call initial routines
	RSPMonitor*		rsp(0);
	TBBMonitor*		tbb(0);
	ECMonitor*     ec(0);
	
	// monitor RSP?
	if (globalParameterSet()->getUint32("WatchRSPboards",0)) {
		rsp = new RSPMonitor("RSPMonitor");
		rsp->start();
		LOG_INFO("Monitoring the RSP boards");
	}

	// monitor TBB?
	if (globalParameterSet()->getUint32("WatchTBboards",0)) {
		tbb = new TBBMonitor("TBBMonitor");
		tbb->start();
		LOG_INFO("Monitoring the TB boards");
	}

	// monitor EC?
	if (globalParameterSet()->getUint32("WatchEnvCntrl",0)) {
		ec = new ECMonitor("ECMonitor");
		ec->start();
		LOG_INFO("Monitoring the Environment Controller");
	}

	// sanity check
	if (!tbb && !rsp && !ec) {
		LOG_FATAL_STR("Non of the monitortask (WatchRSPboards, WatchTBboards, WatchEnvCntrl) "
						"was switched on in the configfile, terminating program");
		return (0);
	}

	// ok, we have something to do, do it.
	GCFScheduler::instance()->setDelayedQuit(true);	// we need a clean shutdown
	GCFScheduler::instance()->run();	// until stop was called

	if (rsp) {
		rsp->quit();		// let task quit nicely
	}
	if (tbb) {
		tbb->quit();		// let task quit nicely
	}
   if (ec) {
		ec->quit();		   // let task quit nicely
	}
	double	postRunTime = globalParameterSet()->getDouble("closingDelay", 1.5);
	GCFScheduler::instance()->run(postRunTime);	// let processes die.

	return (0);
}

