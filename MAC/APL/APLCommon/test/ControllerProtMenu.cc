//#  ControllerProtMenu.cc: Program for testing controller manually
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
//#  $Id: ControllerProtMenu.cc 15192 2010-03-11 11:44:15Z overeem $

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

#include "ControllerProtMenu.h"

using namespace std;

namespace LOFAR {
	using namespace APLCommon;
	using namespace GCF::TM;
	namespace Test {

//
// ControllerProtMenu()
//
ControllerProtMenu::ControllerProtMenu() :
	GCFTask 			((State)&ControllerProtMenu::initial_state,string("TestCtlr")),
	itsListener			(0),
	itsChildPort		(0)
{
	LOG_TRACE_OBJ ("ControllerProtMenu construction");
	
	// create listener port
	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_APLTEST_CTLRMENU, GCFPortInterface::MSPP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsListener, "Can't create a listener port");

	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
}


//
// ~ControllerProtMenu()
//
ControllerProtMenu::~ControllerProtMenu()
{
	LOG_TRACE_OBJ ("~ControllerProtMenu");

}


//
// initial_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::initial_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("initial_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		break;
	  
    case F_INIT:
		// Start ChildControl task
		cout << "WAITING FOR A CONTROLLER TO CONNECT...";
		itsListener->open();
   		break;

	case F_ACCEPT_REQ: {
			itsChildPort = new GCFTCPPort;
			// reminder: init (task, name, type, protocol [,raw])
			itsChildPort->init(*this, "newChild", GCFPortInterface::SPP, CONTROLLER_PROTOCOL);
			itsListener->accept(*itsChildPort);
			cout << endl << "Got a connection, waiting for the controllerName...";
		}
		break;

	case CONTROL_CONNECT: {
			CONTROLConnectEvent msg(event);
				itsControllerName = msg.cntlrName;
				cout << endl << "Name of the controller is " << itsControllerName << endl;
				_doActionMenu();	// does a TRAN
		}
		break;

	case CONTROL_QUITED: {
			CONTROLQuitedEvent msg(event);
			cout << endl << "Connection failed with result = " << msg.result << endl;
			if (msg.result != CONTROL_NO_ERR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::initial, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// claim_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::claim_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("claim_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto CLAIM state..." << endl;
			CONTROLClaimEvent	msg;
			msg.cntlrName = itsControllerName;
			itsChildPort->send(msg);
		}
   		break;

	case F_DISCONNECTED:
		cout << "Client disconnected, quiting." << endl;
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_CLAIMED: {
			CONTROLClaimedEvent msg(event);
			cout << endl << "Claim result = " << msg.result << endl;
			if (msg.result != CONTROL_NO_ERR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::claim, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// prepare_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::prepare_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("prepare_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto PREPARE state..." << endl;;
			CONTROLPrepareEvent	msg;
			msg.cntlrName = itsControllerName;
			itsChildPort->send(msg);
		}
   		break;

	case F_DISCONNECTED:
		cout << "Client disconnected, quiting." << endl;
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_PREPARED: {
			CONTROLPreparedEvent msg(event);
			cout << endl << "Prepare result = " << msg.result << endl;
			if (msg.result != CONTROL_NO_ERR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::prepare, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// run_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::run_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("run_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto RUN state..." << endl;;
			CONTROLResumeEvent	msg;
			msg.cntlrName = itsControllerName;
			itsChildPort->send(msg);
		}
   		break;

	case F_DISCONNECTED:
		cout << "Client disconnected, quiting." << endl;
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_RESUMED: {
			CONTROLResumedEvent msg(event);
			cout << endl << "Resume result = " << msg.result << endl;
			if (msg.result != CONTROL_NO_ERR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::run, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// suspend_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::suspend_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("suspend_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto SUSPEND state..." << endl;;
			CONTROLSuspendEvent	msg;
			msg.cntlrName = itsControllerName;
			itsChildPort->send(msg);
		}
   		break;

	case F_DISCONNECTED:
		cout << "Client disconnected, quiting." << endl;
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_SUSPENDED: {
			CONTROLSuspendedEvent msg(event);
			cout << endl << "Suspend result = " << msg.result << endl;
			if (msg.result != CONTROL_NO_ERR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::suspend, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// release_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::release_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("release_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Requesting controller to goto RELEASED state..." << endl;;
			CONTROLReleaseEvent	msg;
			msg.cntlrName = itsControllerName;
			itsChildPort->send(msg);
		}
   		break;

	case F_DISCONNECTED:
		cout << "Client disconnected, quiting." << endl;
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_RELEASED: {
			CONTROLReleasedEvent msg(event);
			cout << endl << "Release result = " << msg.result << endl;
			if (msg.result != CONTROL_NO_ERR) {
				cout << "Bailing out because of the errors." << endl;
				GCFScheduler::instance()->stop ();
			}
			else {
				_doActionMenu();
			}
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::release, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}

//
// finish_state(event, port)
//
GCFEvent::TResult ControllerProtMenu::finish_state(GCFEvent& event, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR ("finish_state:" << eventName(event));

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY: {
			// Start ChildControl task
			cout << "Telling controller we are FINISHED ..." << endl;;
			CONTROLQuitEvent	msg;
			msg.cntlrName = itsControllerName;
			itsChildPort->send(msg);
		}
   		break;

	case F_DISCONNECTED:
		cout << "Client disconnected, quiting." << endl;
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_QUITED: {
			CONTROLQuitedEvent msg(event);
			if (msg.result != CONTROL_NO_ERR) {
				cout << endl << "WARNING: Finish result = " << msg.result << endl;
			}
			else {
				cout << endl << "Finish result = " << msg.result << endl;
			}
			_doActionMenu();
		}
		break;

	default:
		LOG_DEBUG_STR ("ControllerProtMenu::finish, event " << eventName(event) << " not handled");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// _doActionMenu
//
void ControllerProtMenu::_doActionMenu()
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
			TRAN(ControllerProtMenu::claim_state);
			return;
			break;
		case 'p':
			TRAN(ControllerProtMenu::prepare_state);
			return;
			break;
		case 'R':
			TRAN(ControllerProtMenu::run_state);
			return;
			break;
		case 's':
			TRAN(ControllerProtMenu::suspend_state);
			return;
			break;
		case 'r':
			TRAN(ControllerProtMenu::release_state);
			return;
			break;
		case 'f':
			TRAN(ControllerProtMenu::finish_state);
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

  }; // namespace Test
}; // namespace LOFAR

using namespace LOFAR::GCF::TM;
using namespace LOFAR::Test;

int main(int argc, char* argv[])
{
	GCFScheduler::instance()->init(argc, argv, "ControllerProtMenu");

	ControllerProtMenu	tc;
	tc.start(); // make initial transition

	GCFScheduler::instance()->run();

	return 0;
}

