//
//  VHECRtest.cc: framework for testing the VHECR functionality
//
//  Copyright (C) 2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/lofar_fstream.h>
#include "TBBTrigger.h"
#include "VHECRTask.h"


using namespace LOFAR;
using namespace LOFAR::StationCU;

int main(int argc, char* argv[])
{
	switch (argc) {
	case 2:
		break;
	default:
		cout << "Syntax: " << argv[0] << " triggerfile" << endl;
		cout << "Note: program needs file " << argv[0] << ".log_prop" << " for log-system." << endl;
		return (1);
	}

	string	logFile(argv[0]);
	logFile.append(".log_prop");
	INIT_LOGGER (logFile.c_str());

	ifstream	triggerFile;
	triggerFile.open(argv[1], ifstream::in);
	if (!triggerFile) {
		LOG_FATAL_STR("Cannot open triggerfile " << argv[1]);
		return (1);
	}

	char			triggerLine [4096];
	TBBTrigger		theTrigger;
	VHECRTask		theTask;
	theTrigger.itsFlags = 0;
	// process file
	while (triggerFile.getline (triggerLine, 4096)) {
		LOG_DEBUG_STR("input: " << triggerLine);
		if (sscanf(triggerLine, "%d %u %u %u %u %u %u",
				&theTrigger.itsRcuNr,
				&theTrigger.itsSeqNr,
				&theTrigger.itsTime,
				&theTrigger.itsSampleNr,
				&theTrigger.itsSum,
				&theTrigger.itsNrSamples,
				&theTrigger.itsPeakValue) == 7) {
			// call the VHECR task
			theTask.addTrigger(theTrigger);
		}
		else {	// could not read 7 argments
			LOG_ERROR_STR("Can not interpret line: " << triggerLine);
		}
	}

	triggerFile.close();

	return (0);
}
