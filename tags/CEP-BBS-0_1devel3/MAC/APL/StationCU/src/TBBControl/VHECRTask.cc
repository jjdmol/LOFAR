//#  VHECRTask.cc: Implementation of the VHECR algoritms
//#
//#  Copyright (C) 2007
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
#include <APS/ParameterSet.h>

#include "VHECRTask.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;

namespace LOFAR {
	using namespace ACC::APS;
	namespace StationCU {
	
//
// VHECRTask()
//
VHECRTask::VHECRTask(const string&	cntlrName) :
	GCFTask 			((State)&VHECRTask::initial_state,cntlrName),
	itsTimerPort		(0),
	itsNrTriggers		(0),
	itsInitialized		(false)
{
	LOG_DEBUG ("VHECR construction");

	// Readin some parameters from the ParameterSet. [TEST]
	itsMinResponseTime = globalParameterSet()->getUint32("minResponseTime", 100);
	itsMaxResponseTime = globalParameterSet()->getUint32("maxResponseTime", 500);

	// need port for timers.
	itsTimerPort = new GCFTimerPort(*this, "TimerPort");
}


//
// ~VHECRTask()
//
VHECRTask::~VHECRTask()
{
	LOG_DEBUG ("VHECR destruction");

	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// initial_state(event, port)
//
// Wait till rresponse function is set.
//
GCFEvent::TResult VHECRTask::initial_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("initial:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (event.signal) {
    case F_INIT:
		LOG_INFO_STR ("Waiting for initialisation of responsefunction");
		break;
	  
	case F_TIMER:
		// the 'setSaveFunction initializes our timer
		if (!itsInitialized) {
			LOG_ERROR("Received unexpected timer event, staying in init-state");
		}
		else {
			TRAN (VHECRTask::operational);
		}
		break;

	case F_QUIT:
		TRAN(VHECRTask::quiting_state);
		break;

	default:
		return (GCFEvent::NOT_HANDLED);
		break;
	}

	return (status);
}

//
// setSaveFunction
//
void VHECRTask::setSaveFunction(saveFunctionType	aSaveFunction)
{
	itsSaveFunction = aSaveFunction;
	itsInitialized   = true;

	// wakeup statemachine
	itsTimerPort->setTimer(0.0);
}

//
// addTrigger(trigger)
//
void VHECRTask::addTrigger(const TBBTrigger&	trigger)
{
	// [TEST] The contents of this function is pure test code, it adds the trigger to the command queue.
	uint32		sampleTime = 1000;
	uint32		prePages   = 1;
	uint32		postPages  = 2;
	itsCommandVector.push_back(TBBReadCmd(trigger.itsRcuNr, trigger.itsTime, sampleTime, prePages, postPages));	
	itsNrTriggers++;
}


//
// operational(event, port)
//
// Normal operation state. 
//
GCFEvent::TResult VHECRTask::operational(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("operational:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_QUIT:
		TRAN(VHECRTask::quiting_state);
		break;

	case F_TIMER: {
		// its time to report the store-actions 
		// All code for this event is [TEST] code
		if (!itsCommandVector.empty()) {
			itsSaveFunction(itsCommandVector);			// report that we want everything
			itsCommandVector.clear();					// clear buffer
		}
		// calculate new waittime
		uint32	waitTime(itsMinResponseTime + (random() % (itsMaxResponseTime - itsMinResponseTime)));
		LOG_DEBUG_STR("new waitTime=" << waitTime);
		itsTimerPort->setTimer((1.0*waitTime) / 1000.0);	// initialize timer with new time

		// NOTE: the real code will have to set a timer also in order to return to this place.
		}
		break;

	default:
		return (GCFEvent::NOT_HANDLED);
		break;
	}

	return (status);
}

//
// quiting_state(event, port)
//
// Quiting: send QUITED, wait for answer max 5 seconds, stop
//
GCFEvent::TResult VHECRTask::quiting_state(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR ("quiting:" << eventName(event) << "@" << port.getName());

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_ENTRY: {
		itsTimerPort->cancelAllTimers();
		LOG_INFO_STR("Handled " << itsNrTriggers << " triggers.");	// [TEST]
		break;
	}

	default:
		return (GCFEvent::NOT_HANDLED);
		break;
	}

	return (status);
}

}; // StationCU
}; // LOFAR
