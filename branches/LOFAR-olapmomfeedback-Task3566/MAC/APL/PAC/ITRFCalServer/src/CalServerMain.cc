//#
//#  CalServerMain.cc: implementation of CalServer class
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
//#  Note: this file is formatted with tabstop 4
//#
//#  $Id: CalServer.cc 12256 2008-11-26 10:54:14Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_bitset.h>
#include <Common/ParameterSet.h>
#include <Common/Exception.h>
#include <Common/Version.h>

#include <APL/LBA_Calibration/lba_calibration.h>		// the matlab stuff
#include <MACIO/MACServiceInfo.h>

#include "CalServer.h"
#include "ACCcache.h"
#include "ACMProxy.h"
#include "LBACalibration.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace RTC;
using namespace ICAL;

// Use a terminate handler that can produce a backtrace.
Exception::TerminateHandler t(Exception::terminate);

//
// MAIN
//
int main(int argc, char** argv)
{
	GCFScheduler::instance()->init(argc, argv, "iCalServer");

	LOG_INFO("MACProcessScope: LOFAR_PermSW_CalServer");

	LOG_INFO(formatString("Program %s is starting", argv[0]));

	// First create the LBACalibration instance because that initializes the matlab env.
	LOG_INFO("Creating the LBA calibration component");
	LBACalibration	calibrationModule;
	
	// Allocate buffers for storing ACM results (uses matlab env)
	ACCcache	AccCache;

	calibrationModule.setACCs(AccCache);

	// create CalServer and ACMProxy tasks
	// they communicate via the ACCs instance
	try {
		CalServer cal     ("CalServer", AccCache, calibrationModule);
		ACMProxy  acmproxy("ACMProxy",  AccCache);	// always writes in back buffer.

		cal.start();      // make initial transition
		acmproxy.start(); // make initial transition

		GCFScheduler::instance()->run();
	}
	catch (Exception& e) {
		LOG_ERROR_STR("Exception: " << e.what());
		exit(EXIT_FAILURE);
	}

	LOG_INFO("Normal termination of program");

	return (0);
}
