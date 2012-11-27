//#  HardwareMonitor.cc: Main entry for the HardwareMonitor.
//#
//#  Copyright (C) 2011
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
//#  $Id: HardwareMonitorMain.cc 14858 2010-01-22 09:14:52Z loose $
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>

#include <Common/ParameterSet.h>
#include "BlueGeneMonitor.h"
#include "ClusterMonitor.h"
#include <CEPCU/Package__Version.h>

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::CEPCU;

int main(int argc, char* argv[])
{
	try {
		// args: cntlrname, parentHost, parentService
		GCFScheduler::instance()->init(argc, argv, "CEPHardwareMonitor");

		LOG_INFO("MACProcessScope: LOFAR_PermSW_HardwareMonitor");
		LOG_INFO(Version::getInfo<CEPCUVersion>("CEPHardwareMonitor"));

		// Create tasks and call initial routines
		BlueGeneMonitor*	bgm(0);
		ClusterMonitor*		ctm(0);
		
		// monitor BLUEGENE?
		if (globalParameterSet()->getUint32("WatchBlueGene",0)) {
			bgm = new BlueGeneMonitor("BlueGeneMonitor");
			bgm->start();
			LOG_INFO("Monitoring the BlueGene");
		}

		// monitor CEP2Cluster?
		if (globalParameterSet()->getUint32("WatchCluster",0)) {
			ctm = new ClusterMonitor("ClusterMonitor");
			ctm->start();
			LOG_INFO("Monitoring the Cluster");
		}

		// sanity check
		if (!bgm && !ctm) {
			LOG_FATAL_STR("Non of the monitortask (WatchBlueGene, WatchCluster) "
							"was switched on in the configfile, terminating program");
			return (0);
		}

		// ok, we have something to do, do it.
		GCFScheduler::instance()->setDelayedQuit(true);	// we need a clean shutdown
		GCFScheduler::instance()->run();				// until stop was called

		if (bgm) {
			bgm->quit();		// let task quit nicely
		}
		if (ctm) {
			ctm->quit();		// let task quit nicely
		}
		double	postRunTime = globalParameterSet()->getDouble("closingDelay", 1.5);
		GCFScheduler::instance()->run(postRunTime);	// let processes die.
	} catch( Exception &ex ) {
		LOG_FATAL_STR("Caught exception: " << ex);
		return 1;
	}

	return (0);
}
