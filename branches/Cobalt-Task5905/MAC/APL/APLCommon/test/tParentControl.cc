//#  tParentControl.cc: Program for testing parentControl
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
//#  $Id: tParentControl.cc 13113 2009-04-16 12:30:06Z overeem $
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

#include <Common/ParameterSet.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
//#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/ParentControl.h>
#include <Controller_Protocol.ph>
#include <StartDaemon_Protocol.ph>

#include "tParentControl.h"

using namespace LOFAR::GCF::TM;

namespace LOFAR {
	namespace APLCommon {

#define FAKEPARENTCTRLSERVICENAME	"FakeParentController:v1.0"
	
//
// TestPC()
//
TestPC::TestPC() :
	TestTask 					((State)&TestPC::initial_state,"MainTestTask"),
	itsParentControlTask		(0),
	itsParentTaskPort			(0),
	itsTimerPort				(0),
	itsFakeParentCtlrListener	(0),
	itsFakeStartDaemonListener	(0)
{
	LOG_DEBUG ("TestPC construction");

	// attach to parent control task
	itsParentControlTask = ParentControl::instance();

	// create listener port that will fake the parent CONTROLLER of the parentControlTask
	itsFakeParentCtlrListener = new GCFTCPPort(*this, FAKEPARENTCTRLSERVICENAME, GCFPortInterface::MSPP, CONTROLLER_PROTOCOL);

	// create listener port that will fake the CTStartDaemon
	itsFakeStartDaemonListener = new GCFTCPPort(*this, MAC_SVCMASK_STARTDAEMON, GCFPortInterface::MSPP, STARTDAEMON_PROTOCOL);

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL,  CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_STRINGS);
}


//
// ~TestPC()
//
TestPC::~TestPC()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
}


//
// initial_state(event, port)
//
// Setup all connections.
//
GCFEvent::TResult TestPC::initial_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		// first setup listener to fake the startdaemon
		itsFakeStartDaemonListener->open();
		itsTimerPort->setTimer(5.0);	// allow init process 5 seconds to open the communication
	break;

	case F_CONNECTED:
		if (&port == itsFakeStartDaemonListener) {
			LOG_INFO("Listener for fake CTStartDaemon started");
			itsFakeParentCtlrListener->open();
		}
		if (&port == itsFakeParentCtlrListener) {
			LOG_INFO("Listener for fake ParentController started");
			itsParentTaskPort = itsParentControlTask->registerTask(this);
			itsParentTaskPort->open();
		}
		if (&port == itsParentTaskPort) {
			LOG_INFO("Opened ITC port with parentTask");
			itsTimerPort->cancelAllTimers();
			TRAN(TestPC::emulateStartDaemon);		// go to next state.
		}
		break;

	case F_DISCONNECTED:
		LOG_FATAL_STR("Unexpected disconnect on port " << port.getName());
		GCFScheduler::instance()->stop();
		break;

	case F_ACCEPT_REQ:
		return (GCFEvent::NEXT_STATE);

	case F_TIMER:
		LOG_FATAL("Communicationlines not active within 5 seconds, aborting program!");
		GCFScheduler::instance()->stop();
		break;

	default:
		LOG_DEBUG_STR ("initial::default " << eventName(event) << "@" << port.getName());
		break;
	}    

	return (GCFEvent::HANDLED);
}


//
// emulateStartDaemon(event, port)
//
// Wait for connection from ParentTask, followed by an Announcement event and send newParent message
//
GCFEvent::TResult TestPC::emulateStartDaemon(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("emulateSD:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ACCEPT_REQ: {
		ASSERTSTR(&port == itsFakeStartDaemonListener, "Unexpected connection request on port " << port.getName());
		itsFakeStartDaemonClient = new GCFTCPPort;
		itsFakeStartDaemonClient->init(*this, "StartDaemonClient", GCFPortInterface::SPP, STARTDAEMON_PROTOCOL);
		itsFakeStartDaemonListener->accept(*itsFakeStartDaemonClient);
		LOG_INFO("Created 'StartDaemonClient' port to let ParentTask talk to my fake StartDaemon");
		// wait for connect and announcement event
		break;
	}

	case F_CONNECTED:
		LOG_INFO_STR("Ingoring connected event on port " << port.getName());
		break;

	case F_DISCONNECTED:
		LOG_FATAL_STR("Unexpected disconnect on port " << port.getName());
		GCFScheduler::instance()->stop();
		break;

	case STARTDAEMON_ANNOUNCEMENT: {
		STARTDAEMONAnnouncementEvent	inMsg(event);
		LOG_INFO_STR(port.getName() << ": Received ANNOUNCEMENT(" << inMsg.cntlrName << ")");
		STARTDAEMONNewparentEvent	outMsg;
		outMsg.cntlrName     = inMsg.cntlrName;
		outMsg.parentHost    = myHostname(false);
		outMsg.parentService = FAKEPARENTCTRLSERVICENAME;
		port.send(outMsg);			// should result in CONTROL_CONNECT
		LOG_INFO_STR(port.getName() << ": Sent NEWPARENT(" << inMsg.cntlrName << "," << FAKEPARENTCTRLSERVICENAME << ")");
		break;
	}

	case CONTROL_CONNECT: {
		CONTROLConnectEvent	ccEvent(event);
		LOG_INFO_STR(port.getName() << ": Received CONTROL_CONNECT(" << ccEvent.cntlrName << ")");
		TRAN(TestPC::Wait4Connect2ChildTask);
		break;
	}

	default:
		LOG_DEBUG_STR ("initial::default " << eventName(event) << "@" << port.getName());
		break;
	}

	return (GCFEvent::HANDLED);
}

//
// Wait4Connect2ChildTask(event, port)
//
// Wait for connection from ParentTask, followed by an Announcement event and send newParent message
//
GCFEvent::TResult TestPC::Wait4Connect2ChildTask(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("wait4connect:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ACCEPT_REQ: {
		ASSERTSTR(&port == itsFakeParentCtlrListener, "Unexpected connection request on port " << port.getName());
		itsFakeParentCtlrClient = new GCFTCPPort;
		itsFakeParentCtlrClient->init(*this, "ParentCtlrClient", GCFPortInterface::SPP, CONTROLLER_PROTOCOL);
		itsFakeParentCtlrListener->accept(*itsFakeParentCtlrClient);
		LOG_INFO("Created 'ParentCtlrClient' port to let ParentTask talk to my fake ParentController");
		// wait for connect and announcement event
		break;
	}

	case F_CONNECTED:
		LOG_INFO("ParentControlTask is now connected to fake ParentController");
		LOG_INFO("Complete fake environment is created going to do some test...");
		itsTimerPort->cancelAllTimers();
		TRAN(TestPC::TestFrame);
		break;

	case F_DISCONNECTED:
		LOG_FATAL_STR("Unexpected disconnect on port " << port.getName());
		GCFScheduler::instance()->stop();
		break;

	case CONTROL_CONNECT: {
		CONTROLConnectEvent	ccEvent(event);
		LOG_INFO_STR(port.getName() << ": Received CONTROL_CONNECT(" << ccEvent.cntlrName << ")");
		TRAN(TestPC::Wait4Connect2ChildTask);
		break;
	}

	default:
		LOG_DEBUG_STR ("initial::default " << eventName(event) << "@" << port.getName());
		break;
	}

	return (GCFEvent::HANDLED);
}


//
// startTesten()
//
void TestPC::startTesten()
{
	// @fake parent controller@           @main task@
	// recv CONNECT (Main,ok)    <-- PT <-- send CONNECT (Main,ok) 
	// send CONNECTED (TtestPC)  --> PT
	CONTROLConnectedEvent	cc1Event;
	cc1Event.cntlrName = "MainTestTask";
	CONTROLConnectedEvent	cc2Event;
	cc2Event.cntlrName = "TestPC";
	newTest("Test CONNECT - CONNECTED process");
	addSendTest (&cc1Event, 		itsParentTaskPort);
	addRecvTest (CONTROL_CONNECT,	itsFakeParentCtlrClient);
	addSendTest (&cc2Event, 		itsFakeParentCtlrClient);
	addPause(2.0);

	// @fake parent controller@           @main task@
	// send CLAIM   (Main)    --> PT --> recv CLAIM   (Main)
	// recv CLAIMED (Main,ok) <-- PT <-- send CLAIMED (Main,ok) 
	CONTROLClaimEvent		claim;
	claim.cntlrName = "MainTestTask";
	CONTROLClaimedEvent		claimed;
	claimed.cntlrName = "MainTestTask";
	claimed.result	  = CONTROL_NO_ERR+1;
	newTest("Test CLAIM - CLAIMED process with status ERROR");
	addSendTest (&claim, 			itsFakeParentCtlrClient);
	addRecvTest (CONTROL_CLAIM, 	itsParentTaskPort);
	addSendTest (&claimed, 		itsParentTaskPort);
	addRecvTest (CONTROL_CLAIMED,	itsFakeParentCtlrClient);
	addPause(2.0);

	// @fake parent controller@             @main task@
	// send CLAIM   (Main)       --> PT --> recv CLAIM   (Main)
	// recv CLAIMED (Main,error) <-- PT <-- send CLAIMED (Main,error) 
	claimed.result = CONTROL_NO_ERR;
	newTest("Test CLAIM - CLAIMED process without errors");
	addSendTest (&claim, 			itsFakeParentCtlrClient);
	addRecvTest (CONTROL_CLAIM,	itsParentTaskPort);
	addSendTest (&claimed, 		itsParentTaskPort);
	addRecvTest (CONTROL_CLAIMED,	itsFakeParentCtlrClient);
	addPause(2.0);

	// @fake parent controller@  
	// send CLAIM   (????) --> PT : error unknown controller
	claimed.cntlrName = "UnknownController";
	newTest("Test CLAIMED message with name ERROR");
	addSendTest (&claimed, 	   itsParentTaskPort);
	addPause(2.0);

	// @fake parent controller@           @main task@
	// send RESUME  (Main) --> PT --> recv PREPARE (Main)  !!! ParentTask fills gaps!!!
	// recv PREPARED(Main) <-- PT <-- send PREPARED(Main), 
	//                         PT --> recv RESUME  (Main), 
	// recv RESUMED (Main) <-- PT <-- send RESUMED (Main)
	CONTROLResumeEvent		resume;
	resume.cntlrName = "MainTestTask";
	CONTROLResumedEvent		resumed;
	resumed.cntlrName = "MainTestTask";
	resumed.result	  = CONTROL_NO_ERR;
	CONTROLPreparedEvent	prepared;
	prepared.cntlrName = "MainTestTask";
	prepared.result	  = CONTROL_NO_ERR;
	newTest("Test RESUME - RESUMED before PREPARE WAS SEND");
	addSendTest (&resume, 			itsFakeParentCtlrClient);
	addRecvTest (CONTROL_PREPARE,	itsParentTaskPort);
	addSendTest (&prepared, 		itsParentTaskPort);
	addRecvTest (CONTROL_PREPARED,	itsFakeParentCtlrClient);
	addRecvTest (CONTROL_RESUME,	itsParentTaskPort);
	addSendTest (&resumed,			itsParentTaskPort);
	addRecvTest (CONTROL_RESUMED,	itsFakeParentCtlrClient);
	addPause(2.0);

	// @fake parent controller@           @main task@
	// recv PREPARED(Main) <-- PT <-- send PREPARED(Main), 
	//                         PT --> recv RESUME  (Main), 
	// recv RESUMED (Main) <-- PT <-- send RESUMED (Main)
	prepared.result = CONTROL_NO_ERR;
	newTest("Test out of sync PREPARED message");
	addSendTest (&prepared,   itsParentTaskPort);
	addRecvTest (CONTROL_PREPARED,	itsFakeParentCtlrClient);
	addRecvTest (CONTROL_RESUME,	itsParentTaskPort);
	addSendTest (&resumed,			itsParentTaskPort);
	addRecvTest (CONTROL_RESUMED,	itsFakeParentCtlrClient);
	addPause(2.0);

	doTestSuite();
}

  }; // Test
}; // LOFAR


using namespace LOFAR;
using namespace APLCommon;

//
// main
//
int main(int argc, char* argv[])
{
	GCFScheduler::instance()->init(argc, argv);

	ParentControl*	pc = ParentControl::instance();
	pc->start();	// make initial transition

	TestPC			tpc;
	tpc.start(); 	// make initial transition

	GCFScheduler::instance()->run();

	return 0;
}

