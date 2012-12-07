//#  PythonControl.cc: Implementation of the PythonController task
//#
//#  Copyright (C) 2010-2012
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
#include <Common/LofarLocators.h>

#include <signal.h>
#include <Common/StreamUtil.h>
//#include <Common/lofar_vector.h>
//#include <Common/lofar_string.h>
#include <Common/ParameterSet.h>
#include <Common/ParameterRecord.h>
#include <Common/Exceptions.h>
#include <Common/SystemUtil.h>
#include <Common/hexdump.h>
#include <ApplCommon/LofarDirs.h>
#include <ApplCommon/StationInfo.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVTLogger.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/DP_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/CTState.h>
#include <OTDB/TreeValue.h>

#include "PythonControl.h"
#include "PVSSDatapointDefs.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::posix_time;

namespace LOFAR {
	using namespace APLCommon;
	using namespace GCF::PVSS;
	using namespace GCF::TM;
	using namespace GCF::RTDB;
	using namespace OTDB;
	namespace CEPCU {
	
// static pointer to this object for signal handler
static PythonControl*	thisPythonControl = 0;

//
// PythonControl()
//
PythonControl::PythonControl(const string&	cntlrName) :
	GCFTask 			((State)&PythonControl::initial_state,cntlrName),
	itsPropertySet		(0),
	itsPropertySetInitialized (false),
	itsParentControl	(0),
	itsParentPort		(0),
	itsTimerPort		(0),
	itsListener			(0),
	itsFeedbackListener	(0),					// QUICK FIX #3633
	itsFeedbackPort		(0),					// QUICK FIX #3633
	itsFeedbackResult	(CT_RESULT_NO_ERROR),	// QUICK FIX #3633
	itsPythonPort		(0),
	itsState			(CTState::NOSTATE),
	itsFeedbackFile     (""),
	itsFeedbackWaittime (1.0),
	itsKVTLoggerHost    ("")
//	itsStopTimerID      (0)
{
	LOG_TRACE_OBJ_STR (cntlrName << " construction");

	// First readin our observation related config file.
	LOG_DEBUG_STR("Reading parset file:" << LOFAR_SHARE_LOCATION << "/" << cntlrName);
	globalParameterSet()->adoptFile(string(LOFAR_SHARE_LOCATION)+"/"+cntlrName);

	// Readin some parameters from the conf-file.
	itsFeedbackWaittime = globalParameterSet()->getFloat ("FeedbackWaittime", 1.0);
	itsKVTLoggerHost    = globalParameterSet()->getString("KVTLoggerHost", "localhost");

	// attach to parent control task
	itsParentControl = ParentControl::instance();

	itsListener = new GCFTCPPort (*this, "listener", GCFPortInterface::MSPP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsListener, "Cannot allocate TCP port for server port");

	// QUICK FIX #3633
	itsFeedbackListener = new GCFTCPPort (*this, "Feedbacklistener", GCFPortInterface::MSPP, CONTROLLER_PROTOCOL);
	ASSERTSTR(itsFeedbackListener, "Cannot allocate TCP port for feedback");

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");

	// for debugging purposes
	registerProtocol (CONTROLLER_PROTOCOL, CONTROLLER_PROTOCOL_STRINGS);
	registerProtocol (DP_PROTOCOL, 		   DP_PROTOCOL_STRINGS);
	registerProtocol (KVT_PROTOCOL,		   KVT_PROTOCOL_STRINGS);
}


//
// ~PythonControl()
//
PythonControl::~PythonControl()
{
	LOG_TRACE_OBJ_STR (getName() << " destruction");
	if (itsListener) 
		{ itsListener->close(); delete itsListener; }

	if (itsFeedbackListener) 	// QUICK FIX #3633
		{ itsFeedbackListener->close(); delete itsFeedbackListener; }
}

//
// signalHandler(signum)
//
void PythonControl::signalHandler(int	signum)
{
	LOG_DEBUG (formatString("SIGNAL %d detected", signum));

	if (thisPythonControl) {
		thisPythonControl->finish();
	}
}

//
// finish()
//
void PythonControl::finish()
{
	TRAN(PythonControl::finishing_state);
}


//
// _databaseEventHandler(event)
//
void PythonControl::_databaseEventHandler(GCFEvent& event)
{
	LOG_DEBUG_STR ("_databaseEventHandler:" << eventName(event));

	switch(event.signal) {

	case DP_CHANGED: {
		// TODO: implement something usefull.
		break;
	}  

	default:
		break;
	}  
}

//
// _startPython(execName, ObsNr, parentHost, parentservice)
//
bool PythonControl::_startPython(const string&	pythonProg,
								 int			obsID,
							 	 const string&	pythonHost,
							 	 const string&	parentService)
{
	bool	onRemoteMachine(pythonHost != myHostname(false)  && pythonHost != myHostname(true));

	// Copy observation parset to another machine if neccesary
	string	parSetName(observationParset(obsID));
	if (onRemoteMachine) {
		if (remoteCopy(parSetName, pythonHost, parSetName) != 0) {
			return (false);
		}
	}

	// extra check if we run local.
	ProgramLocator		PL;
	string	executable = PL.locate(pythonProg);
	if (onRemoteMachine) {
		executable = pythonProg;
	}
	else {
		if (executable.empty()) {
			LOG_ERROR_STR("Executable '" << pythonProg << "' not found.");
			return (false);
		}
	}

	// Readin parameters from the obsercationfile.
	itsFeedbackFile = observationParset(obsID)+"_feedback";
	LOG_INFO_STR ("Expect metadata to be in file " << itsFeedbackFile);

	// construct system command
	string	startCmd;
	string	startScript("startPython.sh");
	itsPythonName = formatString("PythonServer{%d}@%s", obsID, pythonHost.c_str());

	if (onRemoteMachine) {
		startCmd = formatString("ssh %s %s %s %s %s %s %s", 
								pythonHost.c_str(), startScript.c_str(),
								executable.c_str(), parSetName.c_str(),
								myHostname(true).c_str(), parentService.c_str(), itsPythonName.c_str());
	}
	else {
		startCmd = formatString("%s %s %s %s %s %s", 
								startScript.c_str(),
								executable.c_str(), parSetName.c_str(),
								myHostname(true).c_str(), parentService.c_str(), itsPythonName.c_str());
	}
	LOG_INFO_STR("About to start: " << startCmd);
#if 1
	FILE*	pipe = popen(startCmd.c_str(), "r");
	if (!pipe) {
		LOG_FATAL_STR("Couldn't execute '" << startCmd << ", errno = " << strerror(errno));
		return (false);
	}
	LOG_INFO("Output of command: ...");
	while (!feof(pipe)) {
		char	buffer[1024];
		LOG_INFO_STR(fgets(buffer, 1024, pipe));
	}
	LOG_INFO("... end of command output");
	int	result = pclose(pipe);
#else
	int	result = system (startCmd.c_str());
#endif
	LOG_INFO_STR ("Result of command = " << result);

	if (result == -1) {
		return (false);
	}
	
	return (true);
}



//
// initial_state(event, port)
//
// Setup all connections.
//
GCFEvent::TResult PythonControl::initial_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
	case F_ENTRY:
		itsListener->open();	// will result in F_CONN
		// QUICK FIX #3633
		itsFeedbackListener->setPortNumber(MAC_PYTHON_FEEDBACK_QF + getObservationNr(getName())%1000);
		itsFeedbackListener->open();	// will result in F_CONN
   		break;

    case F_INIT: {
#ifdef USE_PVSS_DATABASE
		// Get access to my own propertyset.
//		uint32	obsID = globalParameterSet()->getUint32("Observation.ObsID");
		string	propSetName(createPropertySetName(PSN_PYTHON_CONTROL, getName()));
		LOG_DEBUG_STR ("Activating PropertySet: "<< propSetName);
		itsPropertySet = new RTDBPropertySet(propSetName,
											 PST_PYTHON_CONTROL,
											 PSAT_RW,
											 this);
		}
		break;

	case DP_CREATED: {
		// NOTE: this function may be called DURING the construction of the PropertySet.
		// Always exit this event in a way that GCF can end the construction.
		DPCreatedEvent  dpEvent(event);
		LOG_DEBUG_STR("Result of creating " << dpEvent.DPname << " = " << dpEvent.result);
		itsTimerPort->cancelAllTimers();
		itsTimerPort->setTimer(0.0);
		}
		break;

	case F_TIMER: {	// must be timer that PropSet is online.
		// update PVSS.
		LOG_TRACE_FLOW ("Updateing state to PVSS");
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("initial"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
#endif	  
		// Start ParentControl task
		LOG_DEBUG ("Enabling ParentControl task");
		itsParentPort = itsParentControl->registerTask(this);	 // results in CONTROL_CONNECT
		}
		break;

	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		itsMyName  = msg.cntlrName;

		// request from parent task to start up the child side.
		ParameterSet*   thePS  = globalParameterSet();      // shortcut to global PS.
		string  myPrefix        (thePS->locateModule("PythonControl")+"PythonControl.");
		string	pythonProg      (thePS->getString(myPrefix+"pythonProgram",  "@pythonProgram@"));
		string	pythonHost      (thePS->getString(myPrefix+"pythonHost",     "@pythonHost@"));
		itsChildCanCommunicate = thePS->getBool  (myPrefix+"canCommunicate", true);
		// START PYTHON
		bool	startOK = _startPython(pythonProg, getObservationNr(getName()), realHostname(pythonHost), itsListener->makeServiceName());
		if (!startOK) {
			LOG_ERROR("Failed to start the Python environment.");
			CONTROLConnectedEvent	answer;
			answer.cntlrName = msg.cntlrName;
			answer.result    = CONTROL_LOST_CONN_ERR;
			port.send(answer);
			TRAN(PythonControl::finishing_state);
		}
		else {
			if (itsChildCanCommunicate) {
				LOG_DEBUG ("Started Python environment, going to waitForConnection state");
				TRAN(PythonControl::waitForConnection_state);
			}
			else {
				LOG_WARN ("Started Python environment, CHILD CANNOT COMMUNICATE, FAKING RESPONSES!!!");
				CONTROLConnectedEvent	answer;
				answer.cntlrName = itsMyName;
				answer.result = CT_RESULT_NO_ERROR;
				itsParentPort->send(answer);
				TRAN(PythonControl::operational_state);
			}
		}
	}
	break;

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		port.close();
		break;
	
	default:
		LOG_DEBUG_STR ("initial, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    
	return (status);
}


//
// waitForConnection_state(event, port)
//
// The only target in this state is to wat for the CONNECT msg of the start Python env.
//
GCFEvent::TResult PythonControl::waitForConnection_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("PythonControl::waitForConnection_state: " << eventName(event) << "@" << port.getName());
  
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort->cancelAllTimers();	// just to be sure.
		itsTimerPort->setTimer(30.0);		// max waittime for Python framework to respond.
		break;

	case F_TIMER: {
		LOG_FATAL("Python environment does not respond! QUITING!");
		CONTROLConnectedEvent	answer;
		answer.cntlrName = itsMyName;
		answer.result    = CONTROL_LOST_CONN_ERR;
		itsParentPort->send(answer);
		TRAN(PythonControl::finishing_state);
	}

	case F_EXIT:
		itsTimerPort->cancelAllTimers();
		break;
		
	case F_ACCEPT_REQ: {
		itsPythonPort = new GCFTCPPort();
		itsPythonPort->init(*this, "client", GCFPortInterface::SPP, CONTROLLER_PROTOCOL);
		if (!itsListener->accept(*itsPythonPort)) {
			delete itsPythonPort;
			itsPythonPort = 0;
			LOG_ERROR("Connection with Python environment FAILED");
		}
		else {
			LOG_INFO("Connection with unknown client made, waiting for identification");
		}
	}
	break;

	case CONTROL_CONNECT: {
		// acknowledgement from python it is up and running.
		CONTROLConnectEvent		ccEvent(event);
		itsPythonName = ccEvent.cntlrName;
		LOG_INFO_STR("Python identified itself as " << itsPythonName << ". Going to operational state.");
		// inform python
		LOG_INFO_STR("Sending CONNECTED(" << itsPythonName << ") to python");
		CONTROLConnectedEvent	answer;
		answer.cntlrName = itsPythonName;
		answer.result = CT_RESULT_NO_ERROR;
		itsPythonPort->send(answer);
		// inform Parent
		LOG_INFO_STR("Sending CONNECTED(" << itsMyName << ") to Parent");
		answer.cntlrName = itsMyName;
		itsParentPort->send(answer);
		TRAN(PythonControl::operational_state);
	}
	break;
  
	case F_DISCONNECTED:		// Must be from itsListener port
		if (&port == itsPythonPort) {
			LOG_INFO("Unknown client broke connection, waiting for new client");
			itsPythonPort->close();
			delete itsPythonPort;
			itsPythonPort = 0;
		}
		else {
			LOG_ERROR("Disconnect of unexpected port");
		}
		break;

	default:
		LOG_DEBUG("PythonControl::waitForConnection_state, default");
		break;
	}    

	return (status);
}
//
// operational_state(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult PythonControl::operational_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
#ifdef USE_PVSS_DATABASE
		// update PVSS
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("active"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
#endif
	}
	break;

	// QUICK FIX #3633
	case F_ACCEPT_REQ: {
		ASSERTSTR(&port == itsFeedbackListener, "Incoming connection on main listener iso feedbackListener");
		itsFeedbackPort = new GCFTCPPort();
		itsFeedbackPort->init(*this, "feedback", GCFPortInterface::SPP, 0, true);	// raw port
		if (!itsFeedbackListener->accept(*itsFeedbackPort)) {
			delete itsFeedbackPort;
			itsFeedbackPort = 0;
			LOG_ERROR("Connection with Python feedback FAILED");
		}
		else {
			LOG_INFO("Connection made on feedback port, accepting commands");
		}
	} break;

	case F_DISCONNECTED: {
		port.close();
		if (&port == itsPythonPort) {
			LOG_FATAL_STR("Lost connection with Python, going to wait for a new connection");
			TRAN(PythonControl::waitForConnection_state);
		}
		// QUICK FIX #3633
		if (&port == itsFeedbackPort) {
			LOG_FATAL_STR("Lost connection with Feedback of PythonFramework.");
			delete itsFeedbackPort;
			itsFeedbackPort = 0;
		}
	} break;
	
	// QUICK FIX #3633
	case F_DATAIN: {
		ASSERTSTR(&port == itsFeedbackPort, "Didn't expect raw data on port " << port.getName());
		char	buf[1024];
		ssize_t	btsRead = port.recv((void*)&buf[0], 1023);
		buf[btsRead] = '\0';
		string	s;
		hexdump(s, buf, btsRead);
		LOG_INFO_STR("Received command on feedback port: " << s);

		if (!strcmp(buf, "ABORT")) {
			itsFeedbackResult = CT_RESULT_PIPELINE_FAILED;
			TRAN(PythonControl::finishing_state);
		}
		else if (!strcmp(buf, "FINISHED")) {
			TRAN(PythonControl::finishing_state);
		}
		else {
			LOG_ERROR_STR("Received command on feedback port unrecognized");
		}
	} break;

	case DP_CHANGED:
		_databaseEventHandler(event);
		break;

	case F_TIMER:  {
//		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
//		if (timerEvent.id == itsStopTimerID) {
//			LOG_DEBUG("StopTimer expired, starting QUIT sequence");
//			startNewState(CTState::QUIT, ""/*options*/);
//		}
//		else {
//			LOG_WARN_STR("Received unknown timer event");
//		}
	}
	break;

	// -------------------- EVENTS RECEIVED FROM PARENT CONTROL --------------------
	case CONTROL_CONNECT: {
		CONTROLConnectEvent		msg(event);
		LOG_DEBUG_STR("Received CONNECT(" << msg.cntlrName << "), IGNORING!");
		break;
	}

	case CONTROL_SCHEDULED: {
		CONTROLScheduledEvent		msg(event);
		LOG_DEBUG_STR("Received SCHEDULED(" << msg.cntlrName << "), IGNORING!");
		// TODO: do something usefull with this information!
		break;
	}

	case CONTROL_CLAIM: {
		CONTROLClaimEvent		msg(event);
		LOG_DEBUG_STR("Received CLAIM(" << msg.cntlrName << ")");
		if (itsChildCanCommunicate) {
			itsPythonPort->send(msg);
		}
		else {
			LOG_WARN("Sending FAKE Claim response");
			sendControlResult(*itsParentPort, event.signal, itsMyName, CT_RESULT_NO_ERROR);
		}
		break;
	}

	case CONTROL_PREPARE: {
		CONTROLPrepareEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARE(" << msg.cntlrName << ")");
		if (itsChildCanCommunicate) {
			itsPythonPort->send(msg);
		}
		else {
			LOG_WARN("Sending FAKE Prepare response");
			sendControlResult(*itsParentPort, event.signal, itsMyName, CT_RESULT_NO_ERROR);
		}
		break;
	}

	case CONTROL_RESUME: {
		CONTROLResumeEvent		msg(event);
		LOG_DEBUG_STR("Received RESUME(" << msg.cntlrName << ")");
		if (itsChildCanCommunicate) {
			itsPythonPort->send(msg);
		}
		else {
			LOG_WARN("Sending FAKE Resume response");
			sendControlResult(*itsParentPort, event.signal, itsMyName, CT_RESULT_NO_ERROR);
		}
		break;
	}

	case CONTROL_SUSPEND: {
		CONTROLSuspendEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPEND(" << msg.cntlrName << ")");
		if (itsChildCanCommunicate) {
			itsPythonPort->send(msg);
		}
		else {
			LOG_WARN("Sending FAKE Suspend response");
			sendControlResult(*itsParentPort, event.signal, itsMyName, CT_RESULT_NO_ERROR);
		}
		break;
	}

	case CONTROL_RELEASE: {
		CONTROLReleaseEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASE(" << msg.cntlrName << ")");
		if (itsChildCanCommunicate) {
			itsPythonPort->send(msg);
		}
		else {
			LOG_WARN("Sending FAKE Release response");
			sendControlResult(*itsParentPort, event.signal, itsMyName, CT_RESULT_NO_ERROR);
		}
		break;
	}

	case CONTROL_QUIT: {
		CONTROLQuitEvent		msg(event);
		LOG_DEBUG_STR("Received QUIT(" << msg.cntlrName << ")");
		if (itsChildCanCommunicate) {
			itsPythonPort->send(msg);
		}
		else {
			LOG_WARN("Sending FAKE Quit response and quiting application");
//			sendControlResult(*itsParentPort, event.signal, itsMyName, CT_RESULT_NO_ERROR);
			TRAN(PythonControl::finishing_state);
		}
		break;
	}

	// -------------------- RECEIVED FROM PYTHON SIDE --------------------
	case CONTROL_CLAIMED: {
		CONTROLClaimedEvent		msg(event);
		LOG_DEBUG_STR("Received CLAIMED(" << msg.cntlrName << ")");
		msg.cntlrName = itsMyName;
		msg.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);
		break;
	}

	case CONTROL_PREPARED: {
		CONTROLPreparedEvent		msg(event);
		LOG_DEBUG_STR("Received PREPARED(" << msg.cntlrName << ")");
		msg.cntlrName = itsMyName;
		msg.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);
		break;
	}

	case CONTROL_RESUMED: {
		CONTROLResumedEvent		msg(event);
		LOG_DEBUG_STR("Received RESUMED(" << msg.cntlrName << ")");
		msg.cntlrName = itsMyName;
		msg.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);
		break;
	}

	case CONTROL_SUSPENDED: {
		CONTROLSuspendedEvent		msg(event);
		LOG_DEBUG_STR("Received SUSPENDED(" << msg.cntlrName << ")");
		msg.cntlrName = itsMyName;
		msg.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);
		break;
	}

	case CONTROL_RELEASED: {
		CONTROLReleasedEvent		msg(event);
		LOG_DEBUG_STR("Received RELEASED(" << msg.cntlrName << ")");
		msg.cntlrName = itsMyName;
		msg.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);
		break;
	}

	case CONTROL_QUITED: {
		CONTROLQuitedEvent		msg(event);
		LOG_DEBUG_STR("Received QUITED(" << msg.cntlrName << ")");
//		msg.cntlrName = itsMyName;
//		msg.result = CT_RESULT_NO_ERROR;
//		itsParentPort->send(msg);
		LOG_INFO("Python environment has quited, quiting too.");
		TRAN(PythonControl::finishing_state);
		break;
	}

	default:
		LOG_DEBUG("operational_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
// finishing_state(event, port)
//
// Pickup Metadata feedbackfile is any and pass it to SAS
//
GCFEvent::TResult PythonControl::finishing_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("finishing:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		// update PVSS

#ifdef USE_PVSS_DATABASE
		itsPropertySet->setValue(PN_FSM_CURRENT_ACTION, GCFPVString("finished"));
		itsPropertySet->setValue(PN_FSM_ERROR, GCFPVString(""));
#endif
		// Give python environment time to construct the file
		itsTimerPort->setTimer(itsFeedbackFile.empty() ? 3.0 : itsFeedbackWaittime);
		break;
	}

	case F_TIMER: {
		_passMetadatToOTDB();
		CONTROLQuitedEvent		msg;
		msg.cntlrName = itsMyName;
		msg.result = CT_RESULT_NO_ERROR;
		itsParentPort->send(msg);
		GCFScheduler::instance()->stop();
		break;
	}

	case F_DISCONNECTED:
		port.close();
		break;
	
	default:
		LOG_DEBUG("finishing_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}

//
//		_passMetadatToOTDB();
//
void PythonControl::_passMetadatToOTDB()
{
	// No name specified?
	if (itsFeedbackFile.empty()) {
		return;
	}

	// Copy file form remote system to localsystem
	ParameterSet*   thePS  = globalParameterSet();      // shortcut to global PS.
	string  myPrefix  (thePS->locateModule("PythonControl")+"PythonControl.");
	string	pythonHost(thePS->getString(myPrefix+"pythonHost","@pythonHost@"));
	try {
		if (copyFromRemote(realHostname(pythonHost), itsFeedbackFile, itsFeedbackFile) != 0) {
			LOG_ERROR_STR("Failed to copy metadatafile " << itsFeedbackFile << " from host " << realHostname(pythonHost));
			return;
		}
	}
	catch (...) { 
		return;
	}

	// read parameterset
	ParameterSet	metadata;
	metadata.adoptFile(itsFeedbackFile);

	// Try to setup the connection with the database
	string	confFile = globalParameterSet()->getString("OTDBconfFile", "SASGateway.conf");
	ConfigLocator	CL;
	string	filename = CL.locate(confFile);
	LOG_DEBUG_STR("Trying to read database information from file " << filename);
	ParameterSet	otdbconf;
	otdbconf.adoptFile(filename);
	string database = otdbconf.getString("SASGateway.OTDBdatabase");
	string dbhost   = otdbconf.getString("SASGateway.OTDBhostname");
	OTDBconnection  conn("paulus", "boskabouter", database, dbhost);
	if (!conn.connect()) {
		LOG_FATAL_STR("Cannot connect to database " << database << " on machine " << dbhost);
		return;
	}
	LOG_INFO_STR("Connected to database " << database << " on machine " << dbhost);

	TreeValue   tv(&conn, getObservationNr(getName()));

	// Loop over the parameterset and send the information to the KVTlogger.
	// During the transition phase from parameter-based to record-based storage in OTDB the
	// nodenames ending in '_' are implemented both as parameter and as record.
	ParameterSet::iterator		iter = metadata.begin();
	ParameterSet::iterator		end  = metadata.end();
	while (iter != end) {
		string	key(iter->first);	// make destoyable copy
		rtrim(key, "[]0123456789");
//		bool	doubleStorage(key[key.size()-1] == '_');
		bool	isRecord(iter->second.isRecord());
		//   isRecord  doubleStorage
		// --------------------------------------------------------------
		//      Y          Y           store as record and as parameters
		//      Y          N           store as parameters
		//      N          *           store parameter
		if (!isRecord) {
			LOG_DEBUG_STR("BASIC: " << iter->first << " = " << iter->second);
			tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
		}
		else {
//			if (doubleStorage) {
//				LOG_DEBUG_STR("RECORD: " << iter->first << " = " << iter->second);
//				tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
//			}
			// to store is a node/param values the last _ should be stipped of
			key = iter->first;		// destroyable copy
//			string::size_type pos = key.find_last_of('_');
//			key.erase(pos,1);
			ParameterRecord	pr(iter->second.getRecord());
			ParameterRecord::const_iterator	prIter = pr.begin();
			ParameterRecord::const_iterator	prEnd  = pr.end();
			while (prIter != prEnd) {
				LOG_DEBUG_STR("ELEMENT: " << key+"."+prIter->first << " = " << prIter->second);
				tv.addKVT(key+"."+prIter->first, prIter->second, ptime(microsec_clock::local_time()));
				prIter++;
			}
		}
		iter++;
	}
	LOG_INFO_STR(metadata.size() << " metadata values send to SAS");
}

}; // CEPCU
}; // LOFAR
