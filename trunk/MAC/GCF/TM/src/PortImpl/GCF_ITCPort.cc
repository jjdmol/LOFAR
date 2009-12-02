//#  GCF_ITCPort.cc: Direct communication between two tasks in the same process
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
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_ITCPort.h>

namespace LOFAR {
  using MACIO::GCFEvent;
  namespace GCF {
    namespace TM {

//
// GCFITCPort(slavetask, thistask, name, type, protocol)
//
GCFITCPort::GCFITCPort (GCFTask& 		containerTask, 
						GCFTask& 		slaveTask, 
						const string& 	name, 
						TPortType 		type, 
						int 			protocol) : 
    GCFRawPort(containerTask, name, type, protocol), 
    itsSlaveTask(slaveTask),
    itsToSlaveTimerId(),
    itsToContainerTimerId()
{
}

//
// ~GCFITCPort()
//
GCFITCPort::~GCFITCPort()
{
}

//
// open()
//
bool GCFITCPort::open()
{
	schedule_connected();
	return (true);
}

//
// close()
//
bool GCFITCPort::close()
{
	schedule_disconnected();
	return (true);
}

//
// send(event)
//
ssize_t GCFITCPort::send(GCFEvent& e)
{
	// Note: The send and sendBack functions MUST store the events in packed form since
	//       the user will automatically call unpack() when it convert the generic event
	//       into the specialize event: myEventType		myEvent(event)

#if 0
	// send event using a timer event to exit the sending tasks event loop
	uint32		requiredLength;
	char* 		packedBuffer = (char*)e.pack(requiredLength);
	char* 		pEventBuffer = new char[requiredLength]; // is freed in timer handler
	memcpy(pEventBuffer, packedBuffer, requiredLength);
	long timerId = setTimer(0, 0, 0, 0, (void*)pEventBuffer);
#else
	e.pack();
	GCFEvent*	eventCopy = new GCFEvent(e.signal);
	ASSERTSTR(eventCopy, "can't allocate a new event");
	eventCopy->length  = e.length;	// copy settings of the packedbuffer
	eventCopy->_buffer = e._buffer;
	e.length  = 0;	// remove them from the original
	e._buffer = 0;
	long timerId = setTimer(0, 0, 0, 0, (void*)eventCopy);
#endif
	itsToSlaveTimerId.insert(timerId);

	return (timerId);
}

//
// sendBack(event)
//
ssize_t GCFITCPort::sendBack(GCFEvent& e)
{
	e.pack();
	GCFEvent*	eventCopy = new GCFEvent(e.signal);
	ASSERTSTR(eventCopy, "can't allocate a new event");
	eventCopy->length  = e.length;	// copy settings of the packedbuffer
	eventCopy->_buffer = e._buffer;
	e.length  = 0;	// remove them from the original
	e._buffer = 0;

	long timerId = setTimer(0, 0, 0, 0, (void*)eventCopy);
	itsToContainerTimerId.insert(timerId);

	return (timerId);
}

//
// recv(buffer, count)
//
ssize_t GCFITCPort::recv(void* /*buf*/, size_t /*count*/)
{
  return (0);
}

//
// dispatch(event)
//
GCFEvent::TResult GCFITCPort::dispatch(GCFEvent& event)
{
	LOG_TRACE_CALC_STR("GCFITCPort::dispatch");
	GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

	if (event.signal == F_TIMER) {
		GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);

		set<long>::iterator clientIt = itsToSlaveTimerId.find(timerEvent.id);
		set<long>::iterator serverIt = itsToContainerTimerId.find(timerEvent.id);

		if (clientIt != itsToSlaveTimerId.end() || serverIt != itsToContainerTimerId.end()) {
#if 0
			// allocate enough memory for the GCFEvent object and all member data
			// note: real event is in timer-argument field.
			char* packedBuffer = (char*)timerEvent.arg;
			uint16 signal;
			uint32 length;
			uint32 gcfeventlen = sizeof(GCFEvent);
			memcpy(&signal,packedBuffer,sizeof(signal));
			memcpy(&length,packedBuffer+sizeof(signal),sizeof(length));
			char *pEventObject = new char[gcfeventlen+length];
			GCFEvent* pActualEvent = (GCFEvent*)pEventObject;
#else
			GCFEvent* pActualEvent = (GCFEvent*)timerEvent.arg;
#endif
			if (pActualEvent!=0) {
#if 0
				pActualEvent->signal = signal;
				pActualEvent->length = length;
				memcpy(pEventObject+gcfeventlen,packedBuffer+sizeof(signal)+sizeof(length),length);
#endif

				// client timer expired? dispatch to slave
				if (clientIt != itsToSlaveTimerId.end()) {
					LOG_TRACE_CALC(formatString("GCFITCPort::dispatch calling clientTask.doEvent, event@%08X", pActualEvent));
					status = itsSlaveTask.doEvent(*pActualEvent, *this);
					// extra check to see if it still exists:
					clientIt = itsToSlaveTimerId.find(timerEvent.id);
					if (clientIt != itsToSlaveTimerId.end()) {
						itsToSlaveTimerId.erase(clientIt);
					}
				}
				// server timer expired? dispatch to server
				else if (serverIt != itsToContainerTimerId.end()) {
					LOG_TRACE_CALC(formatString("GCFITCPort::dispatch calling serverTask.doEvent, event@%08X", pActualEvent));
					LOG_TRACE_CALC_STR("event = " << *pActualEvent);
					status = _pTask->doEvent(*pActualEvent, *this);
					// extra check to see if it still exists:
					serverIt = itsToContainerTimerId.find(timerEvent.id);
					if (serverIt != itsToContainerTimerId.end()) {
						itsToContainerTimerId.erase(serverIt);
					}
				}        
#if 0
				delete[] pEventObject;	// delete the regenerated event
				delete[] packedBuffer;	// delete the buffer tied to the timer.
#else
				LOG_TRACE_CALC(formatString("GCFITCPort::dispatch deleting event attached to timer (%08X)", pActualEvent));
				delete pActualEvent;	// delete the buffer tied to the timer.
				LOG_TRACE_CALC("GCFITCPort::dispatch event deleted");
#endif
			}
		}
	}

	if (status == GCFEvent::NOT_HANDLED) {
		status = GCFRawPort::dispatch(event);
	}

	return (status);
}

    } // namespace TM
  }	// namespace GCF
} // namespace LOFAR
