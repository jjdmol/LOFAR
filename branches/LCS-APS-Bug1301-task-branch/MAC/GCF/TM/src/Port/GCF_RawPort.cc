//#  GCF_RawPort.cc: Raw connection to a remote process
//#
//#  Copyright (C) 2002-2003
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

#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GTM_Defines.h>
#include <Timer/GTM_TimerHandler.h>
#include <Common/ParameterSet.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

// static framework events
static GCFEvent disconnectedEvent(F_DISCONNECTED);
static GCFEvent connectedEvent   (F_CONNECTED);
static GCFEvent closedEvent      (F_CLOSED);

//
// GCFRawPort(task, name, type, protocol, raw)
//
GCFRawPort::GCFRawPort(GCFTask& 	 task, 
                       const string& name, 
                       TPortType 	 type,  
                       int 			 protocol,
                       bool 		 transportRawData) : 
    GCFPortInterface(&task, name, type, protocol, transportRawData),   
    _pMaster(0)
{
	_pTimerHandler = GTMTimerHandler::instance(); 
	ASSERT(_pTimerHandler);
}

//
// GCFRawPort()
//
GCFRawPort::GCFRawPort() :
    GCFPortInterface(0, "", SAP, 0, false),   
    _pMaster(0)
{
	_pTimerHandler = GTMTimerHandler::instance(); 
	ASSERT(_pTimerHandler);
}

//
// GCFRawPort(task, name, type, protocol, raw)
//
void GCFRawPort::init(GCFTask& 		task, 
                      const string& name, 
                      TPortType 	type, 
                      int 			protocol,
                      bool 			transportRawData)
{
    GCFPortInterface::init(task, name, type, protocol, transportRawData);
    _pMaster = 0;
}

//
// ~GCFRawPort()
//
GCFRawPort::~GCFRawPort()
{
	cancelAllTimers();
	ASSERT(_pTimerHandler);
	GTMTimerHandler::release();
	_pTimerHandler = 0;
}

//
// dispatch(event)
//
GCFEvent::TResult GCFRawPort::dispatch(GCFEvent& event)
{
	GCFEvent::TResult status = GCFEvent::NOT_HANDLED;
	ASSERT(_pTask);

	// Test whether the event is a framework event or not
	if ((F_DATAIN != event.signal) && (F_DATAOUT != event.signal) &&
						  (F_EVT_PROTOCOL(event) != F_FSM_PROTOCOL) &&
						  (F_EVT_PROTOCOL(event) != F_PORT_PROTOCOL)) {
		// Inform about the fact of an incomming message
		LOG_DEBUG(formatString ("%s was received on port '%s' in task '%s'",
//								_pTask->eventName(event).c_str(), 
								eventName(event).c_str(), 
								getRealName().c_str(), 
								_pTask->getName().c_str())); 
	}

	switch (event.signal) {
	case F_CONNECTED:
		LOG_TRACE_STAT(formatString ("Port '%s' in task '%s' is connected!",
							getRealName().c_str(), _pTask->getName().c_str()));
		_state = S_CONNECTED;
		break;

	case F_DISCONNECTED: 
	case F_CLOSED:
		LOG_TRACE_STAT(formatString ("Port '%s' in task '%s' is %s!",
					getRealName().c_str(), _pTask->getName().c_str(),
					(event.signal == F_CLOSED ? "closed" : "disconnected"))); 
		_state = S_DISCONNECTED;
		break;

	case F_TIMER: {
		// result of the schedule_* methods
		GCFTimerEvent* pTE = static_cast<GCFTimerEvent*>(&event);
		if (&disconnectedEvent == pTE->arg || &connectedEvent == pTE->arg ||
												&closedEvent == pTE->arg) {    
			return dispatch(*((GCFEvent*) pTE->arg));
		}
		break;
	}

	case F_DATAIN: {
		// the specific transport implementations informs the rawport about 
		// incomming data
		if (!isTransportRawData()) {
			// user of the port wants to receive GCFEvent formed events
			return recvEvent(); // implicits indirect call of this dispatch 
		}
		break;
	}

	default:
	// 170507: This IN/OUT stuff is not very handy in the control-chains.
	//		   It is also not neccesary to check this because signals will always
	//		   be catched in switches. It is much more usefull to use these two bits
	//		   for marking whether a message is an order/request or an acknowlegdement.
	//		   For now these checks are disabled.
#if 0
		if (SPP == getType() && (F_EVT_INOUT(event) == F_OUT)) {    
			LOG_ERROR(formatString ("Developer error in %s (port %s): "
									"received an OUT event (%s) in a SPP",
									_pTask->getName().c_str(), 
									getRealName().c_str(), 
									_pTask->eventName(event)));    
		return (status);
		}
		else if (SAP == getType() && (F_EVT_INOUT(event) == F_IN)) {
			LOG_ERROR(formatString ("Developer error in %s (port %s): "
									"received an IN event (%s) in a SAP",
									_pTask->getName().c_str(), 
									getRealName().c_str(), 
									_pTask->eventName(event)));    
			return (status);
		}
#endif
		break;
	}

	if (isSlave()) {
		_pMaster->setState(_state);
		status = _pTask->dispatch(event, *_pMaster);      
	}
	else {
		status = _pTask->dispatch(event, *this);
	}

	return (status);
}

//
// settimer(sec, usec, itvsec, itvusec, arg)
//
long GCFRawPort::setTimer(long delay_sec, long delay_usec,
						  long interval_sec, long interval_usec,
						  void* arg)
{
	ASSERT(_pTimerHandler);
	uint64 	delay(delay_sec);
	uint64 	interval(interval_sec);
	delay    *= 1000000;
	interval *= 1000000;
	delay    += (uint64) delay_usec;
	interval += (uint64) interval_usec;

	return (_pTimerHandler->setTimer(*this, delay, interval, arg));  
}

//
// setTimer(sec, itvsec, arg)
//
long GCFRawPort::setTimer(double delay_seconds, 
						  double interval_seconds,
						  void* arg)
{
  ASSERT(_pTimerHandler);

  return (_pTimerHandler->setTimer(*this, 
									 (uint64) (delay_seconds * 1000000.0), 
									 (uint64) (interval_seconds * 1000000.0),
									 arg));
}

//
// cancelTimer(timerid, arg)
//
int GCFRawPort::cancelTimer(long timerid, void **arg)
{
  ASSERT(_pTimerHandler);
  return _pTimerHandler->cancelTimer(timerid, arg);
}

//
// cancelAllTimers()
//
int GCFRawPort::cancelAllTimers()
{
  ASSERT(_pTimerHandler);
  return _pTimerHandler->cancelAllTimers(*this);
}

//
// timeLeft(timerid)
//
double GCFRawPort::timeLeft(long timerID)
{
	ASSERT(_pTimerHandler);
	return (_pTimerHandler->timeLeft(timerID));
}

//
// findAddr(addr)
//
bool GCFRawPort::findAddr(TPeerAddr& addr)
{
	// find remote address
	addr.taskname = "";
	addr.portname = "";

	if (_type != SAP) {
		return (false);
	}

	string remoteAddr;
	// try mac.top.<taskname>.<name>.remoteservice in parameterSet
	string remoteAddrParam = formatString(PARAM_SERVER_SERVICE_NAME, 
							_pTask->getName().c_str(), getRealName().c_str());
	if (globalParameterSet()->isDefined(remoteAddrParam)) {
		remoteAddr = globalParameterSet()->getString(remoteAddrParam);
	}
	else {
		LOG_DEBUG(formatString(
						"No remote address found for port '%s' of task '%s'",
						getRealName().c_str(), _pTask->getName().c_str()));
		return (false);
	}

	// format is: "<taskname>:<portname>"
	string::size_type 	colon = remoteAddr.find(":",0);
	if (colon == string::npos) {
		return (false);
	}
	addr.taskname = remoteAddr.substr(0,colon);
	addr.portname = remoteAddr.substr(colon+1);

	return (true);
}

//
// schedule_disconnect()
//
void GCFRawPort::schedule_disconnected()
{
  // forces a context switch
  setTimer(0, 0, 0, 0, (void*)&disconnectedEvent);
}

//
// schedule_close()
//
void GCFRawPort::schedule_close()
{
  // forces a context switch
  setTimer(0, 0, 0, 0, (void*)&closedEvent);
}

//
// schedule_connect()
//
void GCFRawPort::schedule_connected()
{
  // forces a context switch
  setTimer(0, 0, 0, 0, (void*)&connectedEvent);
}

//
// recvEvent()
//
GCFEvent::TResult GCFRawPort::recvEvent()
{
	GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

	GCFEvent e;
	// expects and reads signal
	if (recv(&e.signal, sizeof(e.signal)) != sizeof(e.signal)) {
		// don't continue with receiving
	}
	// expects and reads length
	else if (recv(&e.length, sizeof(e.length)) != sizeof(e.length)) {
		// don't continue with receiving
	}  
	// reads payload if specified
	else if (e.length > 0) {
		GCFEvent* full_event = 0;
		char* event_buf = new char[sizeof(e) + e.length];
		full_event = (GCFEvent*)event_buf;
		memcpy(event_buf, &e, sizeof(e));

		// read the payload right behind the just memcopied basic event struct
		if (recv(event_buf + sizeof(e), e.length) > 0) {          
			// dispatchs an event with just received params
			status = dispatch(*full_event);
		}    
		delete [] event_buf;
	}
	// dispatchs an event without params
	else {
		status = dispatch(e);
	}

	if (status != GCFEvent::HANDLED) {
		ASSERT(getTask());
		LOG_DEBUG(formatString (
			"'%s' for port '%s' in task '%s' not handled or an error occured",
//			getTask()->eventName(e).c_str(), 
			eventName(e).c_str(), 
			getRealName().c_str(),
			getTask()->getName().c_str()));
	}

	return (status);
}

//
// getRealName()
//
// Returns the name of the port. When the current port is a slave from
// a GCFPort the name of the GCFPort is used.
// When the name contains a colon we use makeServiceName to optain
// name is constructed equal to the code in PortItf::makeServiceName().
// Note:
string GCFRawPort::getRealName() const
{
//	return (isSlave() ? _pMaster->getName() : _name);

	GCFPort*	thePort = (isSlave() ? _pMaster : (GCFPort*)(this));
	return (thePort->usesModernServiceName() ? 
						thePort->makeServiceName() : thePort->getName());
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
