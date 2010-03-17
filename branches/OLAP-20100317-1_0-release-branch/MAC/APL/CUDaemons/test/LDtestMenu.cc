//#  LDtestMenu.cc: one line description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include "EventPort.h"
#include <APL/APLCommon/StartDaemon_Protocol.ph>
#include <APL/APLCommon/ControllerDefines.h>

using namespace LOFAR;
using namespace LOFAR::APLCommon;
using namespace LOFAR::CUdaemons;

static	EventPort*		SDport = 0;

//
// doSchedule()
//
void doSchedule() {
	STARTDAEMONCreateEvent		event;
	event.cntlrType			= CNTLRTYPE_OBSERVATIONCTRL;
	event.cntlrName			= "testObservation";
	event.parentHost 		= "localHost";
	event.parentService 	= "MacScheduler:v1.0";

	cout << "send request to create an ObservationController" << endl;

	SDport->send(&event);
	GCFEvent*	answer = SDport->receive();
	cout << "answer.length = " << answer->length << endl;
}

//
// doKill()
//
void doKill() {

}

//
// doStop()
//
void doStop() {

}

//
// showMenu()
//
void showMenu() {
	cout << endl << endl << endl;
	cout << "Commands" << endl;
	cout << "s		Start LD" << endl;
	cout << "k		Kill LD" << endl;
	cout << "S		Stop StartDaemon(?)" << endl << endl;

	cout << "q		Quit this program" << endl << endl;
	
	cout << "Enter the letter of your choice: ";
}

//
// MAIN (param1, param2)
//
int main (int argc, char* argv[]) {

	// Always bring up the logger first
	string progName = basename(argv[0]);
	INIT_LOGGER (progName.c_str());

	// Check invocation syntax
	if (argc < 3) {
		LOG_FATAL_STR ("Invocation error, syntax: " << progName <<
						" hostname portnr(startdaemon)");
		return (-1);
	}

	SDport = new EventPort(argv[1], argv[2]);

	// Tell operator we are trying to start up.
	LOG_INFO_STR("Starting up: " << argv[0] << "(" << argv[1] << ", "
								<< argv[2] << ")");
	try {
		char	aChoice = ' ';
		while (aChoice != 'q') {
			showMenu();
			cin.clear();
			cin >> aChoice;
			switch (aChoice) {
				case 's':	doSchedule();		break;
				case 'k':	doKill();			break;
				case 'S':	doStop();			break;
			}
		}

		LOG_INFO_STR("Shutting down: " << argv[0]);
	}
	catch (LOFAR::Exception& ex) {
		LOG_FATAL_STR("Caught exception: " << ex << endl);
		LOG_FATAL_STR(argv[0] << " terminated by exception!");
		return (1);
	}

	LOG_INFO_STR(argv[0] << " terminated normally");

	return (0);

}
