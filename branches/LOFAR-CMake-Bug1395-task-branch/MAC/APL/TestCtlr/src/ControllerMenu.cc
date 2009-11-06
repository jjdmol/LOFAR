//#  ControllerMenu.cc: Program for testing controller manually
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
#include <Common/SystemUtil.h>

#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>

#include "ControllerMenu.h"

using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace GCF::TM;
	namespace Test {
	
//
// ControllerMenu()
//
ControllerMenu::ControllerMenu(uint32	instanceNr) :
	GCFTask 			((State)&ControllerMenu::initial_state,string("TestCtlr")),
	itsTimerPort		(0),
	itsChildControl		(0),
	itsChildPort		(0),
	itsSecondTimer		(0),
	itsQueuePeriod		(0),
	itsClaimPeriod		(0),
	itsStartTime		(),
	itsStopTime			(),
	itsInstanceNr		(instanceNr)
{
	LOG_TRACE_OBJ ("ControllerMenu construction");

	// attach to child control task
	itsChildControl = ChildControl::instance();
	itsChildPort = new GCFITCPort (*this, *itsChildControl, "childITCport", 
									GCFPortInterface::SAP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsChildPort, "Cannot allocate ITCport for childcontrol");
	itsChildPort->open();		// will result in F_CONNECTED

	// need port for timers
	itsTimerPort = new GCFTimerPort(*this, "Timerport");

	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
}


//
// ~ControllerMenu()
//
ControllerMenu::~ControllerMenu()
{
	LOG_TRACE_OBJ ("~ControllerMenu");

}


//
// initial_state(event, port)
//
GCFEvent::TResult ControllerMenu::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("initial_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		break;
	  
    case F_INIT:
		// Start ChildControl task
		LOG_DEBUG ("Enabling ChildControltask");
		itsChildControl->openService(MAC_SVCMASK_APLTEST_CTLRMENU, itsInstanceNr);
		itsChildControl->registerCompletionPort(itsChildPort);
		cout << "Waiting till other tasks are ready...";
		itsTimerPort->setTimer(2.0);	// give other task some time.
   		break;

	case F_TIMER:
		itsCntlrType = _chooseController();
		_doStartMenu();	
		TRAN(ControllerMenu::startup_state);
		break;

	case F_CONNECTED:
	case F_DISCONNECTED:
		break;
	
	default:
		LOG_DEBUG ("ControllerMenu::initial, default");
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
GCFEvent::TResult ControllerMenu::startup_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("startup_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Starting up controller ... ";
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->startChild(itsCntlrType,
											 itsObsNr,
											 itsInstanceNr,
											 myHostname(false))) {
				cout << "Error during start of controller, bailing out" << endl;
				GCFScheduler::instance()->stop();
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
				cout << endl << "Startdaemon reports succesful startup." << endl;
				cout << "Waiting for connection with controller ..." << endl;
			}
			else {
				cout << endl << "StartDaemon could not start the controller, bailing out." << endl;
				GCFScheduler::instance()->stop();
			}
		}
		break;

	case CONTROL_CONNECTED: {
			CONTROLConnectedEvent msg(event);
			cout << endl << "Connection result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();	// does a TRAN
			}
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::startup, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// claim_state(event, port)
//
GCFEvent::TResult ControllerMenu::claim_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("claim_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto CLAIM state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->requestState(CTState::CLAIMED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				GCFScheduler::instance()->stop();
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
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::claim, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// prepare_state(event, port)
//
GCFEvent::TResult ControllerMenu::prepare_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("prepare_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto PREPARE state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->requestState(CTState::PREPARED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				GCFScheduler::instance()->stop();
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
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::prepare, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// run_state(event, port)
//
GCFEvent::TResult ControllerMenu::run_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("run_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto RUN state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->requestState(CTState::RESUMED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				GCFScheduler::instance()->stop();
			}
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_RESUMED: {
			CONTROLResumedEvent msg(event);
			cout << endl << "Resume result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::run, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// suspend_state(event, port)
//
GCFEvent::TResult ControllerMenu::suspend_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("suspend_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto SUSPEND state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->requestState(CTState::SUSPENDED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				GCFScheduler::instance()->stop();
			}
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_SUSPENDED: {
			CONTROLSuspendedEvent msg(event);
			cout << endl << "Suspend result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::suspend, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// release_state(event, port)
//
GCFEvent::TResult ControllerMenu::release_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("release_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto RELEASED state..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->requestState(CTState::RELEASED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				GCFScheduler::instance()->stop();
			}
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_RELEASED: {
			CONTROLReleasedEvent msg(event);
			cout << endl << "Release result = " << msg.result << endl;
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::release, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// finish_state(event, port)
//
GCFEvent::TResult ControllerMenu::finish_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		break;
	  
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Telling controller we are FINISHED ..." << endl;;
			string		cntlrName = controllerName(itsCntlrType, itsInstanceNr, itsObsNr);
			if (!itsChildControl->requestState(CTState::QUITED, cntlrName, itsObsNr, itsCntlrType)) {
				cout << "Error during state request, bailing out" << endl;
				GCFScheduler::instance()->stop();
			}
		}
   		break;

	case F_TIMER:
	case F_CONNECTED:
	case F_DISCONNECTED:
		break;

	case CONTROL_QUITED: {
			CONTROLQuitedEvent msg(event);
			if (msg.result != CT_RESULT_NO_ERROR) {
				cout << endl << "WARNING: Finish result = " << msg.result << endl;
			}
			else {
				cout << endl << "Finish result = " << msg.result << endl;
			}
			_doActionMenu();
		}
		break;

	default:
		LOG_DEBUG ("ControllerMenu::finish, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// _chooseController
//
int16 ControllerMenu::_chooseController()
{
	cout << endl;
	cout << endl << "Controller types"<< endl;
	cout << endl << "================"<< endl;
	cout << " 1. ObservationControl" << endl;
	cout << " 2. OnlineControl" << endl;
	cout << " 3. OfflineControl" << endl;
//	cout << " 4. BeamDirectionControl" << endl;
//	cout << " 5. RingControl" << endl;
	cout << " 6. StationControl" << endl;
	cout << " 7. ClockControl" << endl;
	cout << " 8. BeamControl" << endl;
	cout << " 9. CalibrationControl" << endl;
	cout << "10. TBBControl" << endl;
//	cout << "11. StationInfraControl" << endl;
	cout << "12. TestController" << endl;
	cout << endl;
	cout << " 0. stop" << endl;

	int		CntlrType(-1);
	while (CntlrType < 0) {
		cout << endl;
		cout << "Type number of your choice: ";
		cin >> CntlrType;
		if (CntlrType < 0 || CntlrType > 12 || (CntlrType > 3 && CntlrType < 6)) {
			cout << endl << "Wrong number, please retry." << endl;
			CntlrType = -1;
		}
	}

	if (CntlrType == 0) {
		GCFScheduler::instance()->stop();
	}
	
	return (CntlrType + 1);
}

//
// _doStartMenu
//
void ControllerMenu::_doStartMenu()
{
	cout << endl;
	cout << "You need an exportFile from OTDB containing an Observation." << endl;
	cout << "Its name has the format /opt/lofar/share/Observation<nr>." << endl;
	string	command("ls -1 /opt/lofar/share/Observation[0-9]*");
	system(command.c_str());

	int32	obsnr(-1);
	while (obsnr < 0) {
		cout << endl;
		cout << "What observationNumber would you like to use? : ";
		cin.clear();
		cin >> obsnr;
		if (obsnr == 0) {
			GCFScheduler::instance()->stop();
			return;
		}
		ifstream	iFile;
		string obsFileName(formatString("/opt/lofar/share/Observation%d", obsnr));
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
void ControllerMenu::_doActionMenu()
{
	cout << endl;
	cout << "Action to perform" << endl;
	cout << "=================" << endl;
	cout << " c Claim" << endl;
	cout << " p Prepare" << endl;
	cout << " R Resume (run)" << endl;
	cout << " s Suspend" << endl;
	cout << " r Release" << endl;
	cout << " f Finish" << endl << endl;
	cout << " q Quit menuprogram" << endl;

	string	command;
	while (command.empty()) {
		cout << endl;
		cout << "Enter Command to send to controller: ";
		cin.clear();
		cin >> command;
		switch (command.c_str()[0]) {
		case 'c':
			TRAN(ControllerMenu::claim_state);
			return;
			break;
		case 'p':
			TRAN(ControllerMenu::prepare_state);
			return;
			break;
		case 'R':
			TRAN(ControllerMenu::run_state);
			return;
			break;
		case 's':
			TRAN(ControllerMenu::suspend_state);
			return;
			break;
		case 'r':
			TRAN(ControllerMenu::release_state);
			return;
			break;
		case 'f':
			TRAN(ControllerMenu::finish_state);
			return;
			break;
		case 'q':
			GCFScheduler::instance()->stop();
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
void ControllerMenu::_connectedHandler(GCFPortInterface& /*port*/)
{
}

//
// _disconnectedHandler(port)
//
void ControllerMenu::_disconnectedHandler(GCFPortInterface& port)
{
	port.close();
}


}; // namespace Test
}; // namespace LOFAR
