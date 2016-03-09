//#
//#  CalServer.cc: implementation of CalServer class
//#
//#  Copyright (C) 2002-2014
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  Note: this file is formatted with tabstop 4
//#
//#  $Id: CalServer.cc 32365 2015-08-31 12:48:00Z mol $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>

#include <ApplCommon/StationConfig.h>
#include <Common/ParameterSet.h>
#include <GCF/TM/GCF_Control.h>
#include <APL/RTCCommon/PSAccess.h>
#include <Cal_Server/Package__Version.h>

#include "CalServer.h"
#include "ACMProxy.h"
#include "ACC.h"

using namespace std;
using namespace blitz;

using namespace LOFAR;
using namespace RTC;
using namespace GCF::TM;
using namespace CAL;

#define NPOL 2
//
// MAIN
//
int main(int argc, char** argv)
{
	GCFScheduler::instance()->init(argc, argv, "CalServer");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_CalServer");

	LOG_INFO(formatString("Program %s has started", argv[0]));

	ACCs* 			accs; // the ACC buffers
	StationConfig	SC;
	accs = new ACCs(GET_CONFIG("CalServer.N_SUBBANDS", i), SC.nrRSPs * NR_ANTENNAS_PER_RSPBOARD, NPOL);

	if (!accs) {
		LOG_FATAL("Failed to allocate memory for the ACC arrays.");
		exit(EXIT_FAILURE);
	}

	// SOMETIMES CALSERVER IS STARTED BEFORE RSPDRIVER IS ON THE AIR THIS SHOULD NOT BE A PROBLEM
	// BUT IT SOMETIMES IS. QAD HACK TO AVOID HANGING CalServer
	sleep(3);

	//
	// create CalServer and ACMProxy tasks
	// they communicate via the ACCs instance
	//
	bool	ACMProxyEnabled(!globalParameterSet()->getBool("CalServer.DisableACMProxy"));
	try {
		CalServer cal     ("CalServer", *accs);
		ACMProxy  acmproxy("ACMProxy",  *accs);

		cal.start();      // make initial transition
		if (ACMProxyEnabled) {
			LOG_INFO("ACMProxy is enabled");
			acmproxy.start(); // make initial transition
		}

		GCFScheduler::instance()->run();
	}
	catch (Exception& e) {
		LOG_ERROR_STR("Exception: " << e.text());
		exit(EXIT_FAILURE);
	}

	delete accs;

	LOG_INFO("Normal termination of program");

	return 0;
}

