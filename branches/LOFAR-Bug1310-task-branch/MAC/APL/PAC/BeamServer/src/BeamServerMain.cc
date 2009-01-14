//#
//#  BeamServerMain.cc: implementation of BeamServer class
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

#include <getopt.h>

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
#include <GCF/TM/GCF_Task.h>

#include <APL/RTCCommon/daemonize.h>
#include "BeamServer.h"

using namespace LOFAR;
using namespace LOFAR::BS;
using namespace LOFAR::GCF::TM;

//
// MAIN
//
int main(int argc, char* argv[])
{
	/* daemonize if required */
	if (argc >= 2) {
		if (!strcmp(argv[1], "-d")) {
			if (0 != daemonize(false)) {
				cerr << "Failed to background this process: " << strerror(errno) << endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	GCFTask::init(argc, argv, "BeamServer");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_BeamServer");
	LOG_INFO(formatString("Program %s has started", argv[0]));

	try {
		ConfigLocator cl;
		globalParameterSet()->adoptFile(cl.locate("RemoteStation.conf"));

		// set global bf_gain
		g_bf_gain = globalParameterSet()->getInt32("BeamServer.BF_GAIN");
	}
	catch (Exception& e) {
		LOG_FATAL_STR("Failed to load configuration files: " << e.text());
		exit(EXIT_FAILURE);
	}

	try {
		BeamServer beamserver("BeamServer", argc, argv);

		beamserver.start(); // make initial transition

		GCFTask::run();
	}
	catch (Exception& e) {
		LOG_FATAL_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}

	LOG_INFO(formatString("Normal termination of program %s", argv[0]));

	return (0);
}
