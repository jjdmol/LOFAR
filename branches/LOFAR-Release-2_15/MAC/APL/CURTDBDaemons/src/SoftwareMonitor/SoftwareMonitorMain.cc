//  SoftwareMonitor.cc: Main entry for the SoftwareMonitor.
//
//  Copyright (C) 2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Version.h>
#include <Common/ParameterSet.h>
#include <Common/Exception.h>

#include <GCF/RTDB/DP_Protocol.ph>
#include "SoftwareMonitor.h"
#include <CURTDBDaemons/Package__Version.h>

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::RTDBDaemons;
using namespace LOFAR::DP_Protocol;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

int main(int argc, char* argv[])
{
	// args: cntlrname, parentHost, parentService
	GCFScheduler::instance()->init(argc, argv, "SoftwareMonitor");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_SoftwareMonitor");
	LOG_INFO(Version::getInfo<CURTDBDaemonsVersion>("SoftwareMonitor"));

	// for debugging purposes
	registerProtocol (DP_PROTOCOL,  DP_PROTOCOL_STRINGS);

	// Create tasks and call initial routines
	SoftwareMonitor*	swm = new SoftwareMonitor("SoftwareMonitor");
	ASSERTSTR(swm, "Can't create an software monitortask");
	swm->start();

	// ok, we have something to do, do it.
	GCFScheduler::instance()->setDelayedQuit(true);	// we need a clean shutdown
	GCFScheduler::instance()->run();	// until stop was called

	swm->quit();		// let task quit nicely

	double	postRunTime = globalParameterSet()->getDouble("closingDelay", 1.5);
	GCFScheduler::instance()->run(postRunTime);	// let processes die.

	return (0);
}

