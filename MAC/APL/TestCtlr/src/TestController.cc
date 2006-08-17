//#  TestController.cc: Program for testing controller manually
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>

#include <APS/ParameterSet.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/Utils.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StationInfo.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>

#include "TestController.h"

using namespace std;

namespace LOFAR {
	using namespace GCFCommon;
	using namespace APLCommon;
	using namespace GCF::Common;
	using namespace GCF::TM;
	using namespace Deployment;
	using namespace ACC::APS;
	namespace Test {
	
//
// TestController()
//
TestController::TestController() :
	GCFTask 			((State)&TestController::initial_state,string("TestCtlr")),
	itsTimerPort		(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsSecondTimer		(0),
	itsQueuePeriod		(0),
	itsClaimPeriod		(0),
	itsStartTime		(),
	itsStopTime			()
{
	LOG_TRACE_OBJ ("TestController construction");

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "childITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// need port for timers
	itsTimerPort = new GCFTimerPort(*this, "Timerport");

	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_signalnames);
}


//
// ~TestController()
//
TestController::~TestController()
{
	LOG_TRACE_OBJ ("~TestController");

}


//
// initial_state(event, port)
//
GCFEvent::TResult TestController::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("initial_state:" << evtstr(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		break;
	  
    case F_INIT:
		// Start ChildControl task
		LOG_DEBUG ("Enabling ChildControltask");
		itsChildControl->openService(MAC_SVCMASK_APLTEST_CTLRMENU, 0);
		itsChildControl->registerCompletionPort(itsChildPort);
		cout << "Waiting till other tasks are ready...";
		itsTimerPort->setTimer(2.0);	// give other task some time.
   		break;

	case F_TIMER:
		itsCntlrType = _chooseController();
		_doStartMenu();	
		TRAN(TestController::startup_state);
		break;

	case F_CONNECTED:
	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG ("TestController::initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// startup_state(event, port)
//
// startup the controller
//
GCFEvent::TResult TestController::startup_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("startup_state:" << evtstr(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Starting up controller ... ";
			string		cntlrName = controllerName(itsCntlrType, 0, itsObsNr);
			if (!itsChildControl->startChild(cntlrName,
											 itsObsNr,
											 itsCntlrType,
											 0,
											 myHostname(false))) {
				cout << "Error during start of controller, bailing out" << endl;
				stop();
			}
			cout << endl << "Startrequest queued, waiting for confirmation...";
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_STARTED: {
			CONTROLStartedEvent msg(event);
			if (msg.successful) {
				cout << endl << "Startdaemon reports succesful startup.";
				cout << "Waiting for connection with controller ...";
			}
			else {
				cout << endl << "StartDaemon could not start the controller, bailing out." << endl;
				stop();
			}
		}
		break;

	case CONTROL_CONNECTED: {
			CONTROLConnectedEvent msg(event);
			cout << endl << "Connection result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				stop ();
			}
			else {
				_doActionMenu();	// does a TRAN
			}
		}
		break;

	default:
		LOG_DEBUG ("TestController::startup, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// claim_state(event, port)
//
GCFEvent::TResult TestController::claim_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("claim_state:" << evtstr(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto CLAIM state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, 0, itsObsNr);
			if (!itsChildControl->requestState(CTState::CLAIMED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				stop();
			}
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_CLAIMED: {
			CONTROLClaimedEvent msg(event);
			cout << endl << "Claim result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("TestController::claim, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// prepare_state(event, port)
//
GCFEvent::TResult TestController::prepare_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("prepare_state:" << evtstr(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto PREPARE state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, 0, itsObsNr);
			if (!itsChildControl->requestState(CTState::PREPARED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				stop();
			}
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_PREPARED: {
			CONTROLPreparedEvent msg(event);
			cout << endl << "Prepare result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("TestController::prepare, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// _chooseController
//
int16 TestController::_chooseController()
{
	cout << endl;
	cout << endl << "Controller types"<< endl;
	cout << endl << "================"<< endl;
	cout << " 1. ObservationControl" << endl;
	cout << " 2. OnlineControl" << endl;
	cout << " 3. OfflineControl" << endl;
//	cout << " 4. BeamDirectionControl" << endl;
//	cout << " 5. RingControl" << endl;
//	cout << " 6. StationControl" << endl;
	cout << " 7. DigitalBoardControl" << endl;
	cout << " 8. BeamControl" << endl;
//	cout << " 9. CalibrationControl" << endl;
//	cout << "10. StationInfraControl" << endl;
	cout << endl;
	cout << " 0. stop" << endl;

	int		CntlrType(-1);
	while (CntlrType < 0) {
		cout << endl;
		cout << "Type number of your choice: ";
		cin >> CntlrType;
		if (CntlrType < 0 || CntlrType > 8 || (CntlrType > 3 && CntlrType < 7)) {
			cout << endl << "Wrong number, please retry." << endl;
			CntlrType = -1;
		}
	}

	if (CntlrType == 0) {
		stop();
	}
	
	return (CntlrType + 1);
}

//
// _doStartMenu
//
void TestController::_doStartMenu()
{
	cout << endl;
	cout << "You need an exportFile from OTDB containing an Observation." << endl;
	cout << "Its name has the format /opt/lofar/share/Observation_<nr>." << endl;
	string	command("ls -1 /opt/lofar/share/Observation_*");
	system(command.c_str());

	int32	obsnr(-1);
	while (obsnr < 0) {
		cout << endl;
		cout << "What observationNumber would you like to use? : ";
		cin.clear();
		cin >> obsnr;
		if (obsnr == 0) {
			stop();
			return;
		}
		ifstream	iFile;
		string obsFileName(formatString("/opt/lofar/share/Observation_%d", obsnr));
		iFile.open(obsFileName.c_str(), ifstream::in);
		if (!iFile) {
			cout << endl << "Cannot open file " << obsFileName << endl;
			cout << endl << "Try again or type 0 to stop" << endl;
			obsnr = -1;
		}
		else {
			iFile.close();
		}
	}
	itsObsNr = obsnr;
}


//
// _doActionMenu
//
void TestController::_doActionMenu()
{
	cout << endl;
	cout << "Action to perform" << endl;
	cout << "=================" << endl;
	cout << " C Claim" << endl;
	cout << " P Prepare" << endl;
	cout << " S Start" << endl;
	cout << " F Finish" << endl << endl;
	cout << " Q Quit program" << endl;

	string	command;
	while (command.empty()) {
		cout << endl;
		cout << "Enter Command to send to controller: ";
		cin.clear();
		cin >> command;
		switch (command.c_str()[0]) {
		case 'C':
			TRAN(TestController::claim_state);
			return;
			break;
		case 'P':
			TRAN(TestController::prepare_state);
			return;
			break;
#if 0
		case 'S':
			TRAN(TestController::start_state);
			return;
			break;
		case 'F':
			TRAN(TestController::finish_state);
			return;
			break;
#endif
		case 'Q':
			stop();
			break;
		default:
			command = "";
			break;
		}
	}
}



//
// _connectedHandler(port)
//
void TestController::_connectedHandler(GCFPortInterface& port)
{
}

//
// _disconnectedHandler(port)
//
void TestController::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // namespace Test
}; // namespace LOFAR
