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

//
// GCFRawPort(task, name, type, protocol, raw)
//
GCFRawPort::GCFRawPort(GCFTask& 	 task, 
                       const string& name, 
                       TPortType 	 type,  
                       int 			 protocol,
                       bool 		 transportRawData) : 
    GCFPortInterface(&task, name, type, protocol, transportRawData),   
    _pMaster(0),
	itsScheduler(0)
{
	_pTimerHandler = GTMTimerHandler::instance(); 
	ASSERTSTR(_pTimerHandler, "Cannot reach the Timer handler");

	itsScheduler = GCFScheduler::instance();
	ASSERTSTR(itsScheduler, "Cannot reach the GCF scheduler");
}

//
// GCFRawPort()
//
GCFRawPort::GCFRawPort() :
    GCFPortInterface(0, "", SAP, 0, false),   
    _pMaster(0)
{
	_pTimerHandler = GTMTimerHandler::instance(); 
	ASSERTSTR(_pTimerHandler, "Cannot reach the Timer handler");

	itsScheduler = GCFScheduler::instance();
	ASSERTSTR(itsScheduler, "Cannot reach the GCF scheduler");
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
	ASSERT(_pTask);

	// Test whether the event is a framework event or not
	if ((F_DATAIN != event.signal) && (F_DATAOUT != event.signal) &&
						  (F_EVT_PROTOCOL(event) != F_FSM_PROTOCOL) &&
						  (F_EVT_PROTOCOL(event) != F_PORT_PROTOCOL)) {
		// Inform about the fact of an incomming message
		LOG_DEBUG(formatString ("%s was received on port '%s' in task '%s'",
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
		break;
	}

	if (isSlave()) {
		_pMaster->setState(_state);
		return(_pTask->doEvent(event, *_pMaster));
	}

	return(_pTask->doEvent(event, *this));
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
// recvEvent()
//
GCFEvent::TResult GCFRawPort::recvEvent()
{
	bool		error (false);
	GCFEvent	e;
	char* 		event_buf(0);
	GCFEvent*	full_event(&e);

	// expects and reads signal
	if (recv(&e.signal, sizeof(e.signal)) != sizeof(e.signal)) {
		error = true;
		// don't continue with receiving
	}
	// expects and reads length
	else if (recv(&e.length, sizeof(e.length)) != sizeof(e.length)) {
		error = true;
		// don't continue with receiving
	}  
	// reads payload if specified
	else if (e.length > 0) {
		event_buf = new char[sizeof(e) + e.length];
		memcpy(event_buf, &e, sizeof(e));
		full_event = (GCFEvent*)event_buf;

		// read the payload right behind the just memcopied basic event struct
		if (recv(event_buf + sizeof(e), e.length) != e.length) {
			error = true;
		}    
	}

	// receive errors?
	if (error) {
		LOG_ERROR_STR("Error during receiption of an event on port " << getRealName());
		if (event_buf) {
			delete [] event_buf;
		}
		return (GCFEvent::NOT_HANDLED);
	}

	// dispatch the event to the task
	itsScheduler->queueEvent(const_cast<GCFTask*>(getTask()), *full_event, this);

	if (event_buf) {
		delete [] event_buf;
	}
	return (GCFEvent::HANDLED);
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
