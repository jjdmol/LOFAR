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
#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
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
    GCFPortInterface(0, "", SAP, 0, false)
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
//    _pMaster = 0;
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
	if ((F_EVT_PROTOCOL(event) != F_FSM_PROTOCOL) && (F_EVT_PROTOCOL(event) != F_PORT_PROTOCOL)) {
		// Inform about the fact of an incomming message
		LOG_DEBUG(formatString ("%s was received on port '%s' in task '%s'",
								eventName(event).c_str(), 
								getRealName().c_str(), 
								_pTask->getName().c_str())); 
	}

	switch (event.signal) {
	case F_DATAIN: {
		// the specific transport implementations informs the rawport about 
		// incomming data
		if (!isTransportRawData()) {
			// user of the port wants to receive GCFEvent formed events
			return recvEvent(); // implicits indirect call of this dispatch 
		}
		break;
	}

	case F_TIMER: {
		GCFTimerEvent	*TEptr = static_cast<GCFTimerEvent*>(&event);
		if (TEptr->userPtr == &itsTimedMsgs) {
			timeoutInfo	ti;
			ti.value = TEptr->userValue;
			_delTimeout(ti.e.seqnr);
			GCFTimeoutEvent	event(ti.e.signal, ti.e.seqnr);
			_pTask->doEvent(event, *this);
			return (GCFEvent::HANDLED);
		}
		break;
	}

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

	default:
		break;
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
// recvEvent()
//
GCFEvent::TResult GCFRawPort::recvEvent()
{
	bool		error (false);
	GCFEvent*	newEvent = new GCFEvent;
	char* 		event_buf(0);

	// expects and reads signal
	if (recv(&newEvent->signal, sizeof(newEvent->signal)) != sizeof(newEvent->signal)) {
		error = true;
		// don't continue with receiving
	}
	// expects and reads seqnr
	if (recv(&newEvent->seqnr, sizeof(newEvent->seqnr)) != sizeof(newEvent->seqnr)) {
		error = true;
		// don't continue with receiving
	}
	// expects and reads length
	else if (recv(&newEvent->length, sizeof(newEvent->length)) != sizeof(newEvent->length)) {
		error = true;
		// don't continue with receiving
	}  
	// reads payload if specified
	else if (newEvent->length > 0) {
		event_buf = new char[GCFEvent::sizePackedGCFEvent + newEvent->length];
		ASSERTSTR(event_buf, "Could not allocate buffer for " << GCFEvent::sizePackedGCFEvent + newEvent->length << " bytes");
		memcpy(event_buf, 						 					   &newEvent->signal, GCFEvent::sizeSignal);
		memcpy(event_buf+GCFEvent::sizeSignal, 					   &newEvent->seqnr , GCFEvent::sizeSeqnr);
		memcpy(event_buf+GCFEvent::sizeSignal+GCFEvent::sizeSeqnr, &newEvent->length, GCFEvent::sizeLength);

		// read the payload right behind the just memcopied basic event struct
		if (recv(event_buf + GCFEvent::sizePackedGCFEvent, newEvent->length) != (ssize_t)newEvent->length) {
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

	// clear timer if any
	if (newEvent->seqnr) {
		if (!_delTimeout(newEvent->seqnr) && F_OUTDIR(newEvent->signal)) {
			LOG_WARN_STR("Event " << eventName(newEvent->signal) << "@" << getName() << " timed out. DELETING answer");
			return (GCFEvent::HANDLED);
		}
	}

	// dispatch the event to the task
	newEvent->_buffer = event_buf;	// attach buffer to event
	itsScheduler->queueEvent(const_cast<GCFTask*>(getTask()), *newEvent, this);
	delete newEvent;

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
	return (usesModernServiceName() ? makeServiceName() : getName());
}

//
// _addTimeout(event, timeout)
//
void GCFRawPort::_addTimeout(GCFEvent&	event, ulong	timeoutMs)
{
	if (!event.seqnr) {
		return;
	}

	timeoutInfo	ti;
	ti.e.signal = event.signal;
	ti.e.seqnr  = event.seqnr;
	// note pass address of itsTimedMsgs so we have a uniq value when a F_TIMER event occurs.
	int	timerID = _pTimerHandler->setTimer(*this, (uint64)(1000.0*timeoutMs), 0, (void*)&itsTimedMsgs, ti.value);

	itsTimedMsgs[event.seqnr]=timerID;
	LOG_TRACE_OBJ_STR("addTimeout[" << getName() << "](" << eventName(event.signal) << "," << event.seqnr << "," << timeoutMs << "ms)->" << timerID);
}

//
// _delTimeout(seqnr)
//
bool GCFRawPort::_delTimeout(uint16	seqnr)
{
	map<uint16, long>::iterator	iter = itsTimedMsgs.find(seqnr);
	if (iter != itsTimedMsgs.end()) {
		LOG_TRACE_OBJ_STR("delTimeout[" << getName() << "](" << seqnr << ")->" << iter->second);
		_pTimerHandler->cancelTimer(iter->second);
		itsTimedMsgs.erase(iter);
		return (true);
	}

	LOG_TRACE_OBJ_STR("delTimeout[" << getName() << "](" << seqnr << ") no match");
	return (false);
}


  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
