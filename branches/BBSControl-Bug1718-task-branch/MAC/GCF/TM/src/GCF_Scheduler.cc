//#  GCF_Task.cc: task class which encapsulates a task and its behaviour as a 
//#  finite state machine (FSM).
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
#include <Common/lofar_fstream.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>
#include <Common/hexdump.h>

#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Handler.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Scheduler.h>

#include <signal.h>
#include <sys/time.h>

namespace LOFAR {
  namespace GCF {
    namespace TM {


// static member initialisation
int 				GCFScheduler::_argc = 0;
char** 				GCFScheduler::_argv = 0;

static const uint	BATCH_SIZE = 3;		// number of message handled before control is returned to other workProcs.
#define MAX2(a,b) ((a) > (b)) ? (a) : (b)
#define MIN2(a,b) ((a) < (b)) ? (a) : (b)

//
// instance()
//
GCFScheduler*	GCFScheduler::instance()
{
    static GCFScheduler scheduler;

    return &scheduler;
}

//
// Default construction
//
GCFScheduler::GCFScheduler() :
	itsDoExit		(false),
	itsDelayedQuit	(false),
	itsIsInitialized(false),
	itsUseQueue		(true),
	itsFrameworkPort(0)
{
 	itsFrameworkPort = new GCFDummyPort(0, "FrameWork", F_FSM_PROTOCOL);
	ASSERTSTR(itsFrameworkPort, "Cannot construct a port for the framework");
}

//
// ~GCFScheduler()
//
GCFScheduler::~GCFScheduler()
{
	delete itsFrameworkPort;

	// clear the maps
}

//
// init(argc, argv, logfile)
//
void GCFScheduler::init(int argc, char** argv, const string&	logfile)
{
	ASSERTSTR(!itsIsInitialized, "Attempt to initialized the GCFScheduler twice");

	_argc = argc;
	_argv = argv;

	// Try to open the log_prop file, if process has its own log_prop file then use it
	// the INIT_LOGGER otherwise use the default mac.log_prop
	ConfigLocator	aCL;
	string 			procName(basename(argv[0]));
	string			logPropFile(procName + ".log_prop");
	// First try logpropfile <task>.log_prop
	if (aCL.locate(logPropFile) == "") {
		// locator could not find it try defaultname
		logPropFile = "mac.log_prop";
	}

	if (logfile.empty()) {
		INIT_LOGGER(aCL.locate(logPropFile).c_str());
		LOG_INFO_STR ("Initialized logsystem with: " << aCL.locate(logPropFile));
	}
	else {
		INIT_VAR_LOGGER(aCL.locate(logPropFile).c_str(), logfile);
		LOG_INFO_STR ("Initialized logsystem with: " << aCL.locate(logPropFile) <<
						"," << logfile);
	}

	// Read in the ParameterSet of the task (<task>.conf)
	ParameterSet*	pParamSet = globalParameterSet();
	string			configFile(aCL.locate(procName + ".conf"));
	if (!configFile.empty()) {
		LOG_INFO_STR ("Using parameterfile: " << configFile);
		pParamSet->adoptFile(configFile);
	}
	else {
		LOG_INFO_STR ("NO DEFAULT PARAMETERSET FOUND");
	}

	registerProtocol(F_FSM_PROTOCOL, F_FSM_PROTOCOL_STRINGS);
	registerProtocol(F_PORT_PROTOCOL,F_PORT_PROTOCOL_STRINGS);

	itsIsInitialized = true;
}

//
// run(secondsToRun)
//
void GCFScheduler::run(double maxSecondsToRun)
{
	// catch terminate signals
	signal(SIGINT,  GCFScheduler::signalHandler);
	signal(SIGTERM, GCFScheduler::signalHandler);
	signal(SIGPIPE, SIG_IGN);

	double	terminateTime(0.0);
	struct timeval	TV;
	if (maxSecondsToRun) {
		LOG_INFO_STR("Program execution stops over " << maxSecondsToRun << " seconds");
		gettimeofday(&TV, 0);
		terminateTime = TV.tv_sec + (TV.tv_usec / 10000000) + maxSecondsToRun;
	}

	LOG_DEBUG("Entering main loop of GCFScheduler");

	// always start with emptying the already queued events
	handleEventQueue();

	// THE MAIN LOOP OF THE MAC PROCESS
	// can only be interrupted/stopped by calling stop or terminating the application
	while (!itsDoExit) {
		// new handlers can be add during processing the workProc
		// thus a temp handler map is used
		itsTempHandlers.clear();
		itsTempHandlers.insert(itsHandlers.begin(), itsHandlers.end()); 
//		LOG_DEBUG_STR("Nr of handlers: " << itsHandlers.size());
		for (HandlerMap_t::iterator iter = itsTempHandlers.begin() ; 
								iter != itsTempHandlers.end() && !itsDoExit; ++iter) {
			if (iter->second) {
				iter->first->workProc();
				handleEventQueue();
			}
		} // for

		// time to quit?
		if (maxSecondsToRun) {
			gettimeofday(&TV, 0);
			if ((TV.tv_sec + (TV.tv_usec / 10000000)) >= terminateTime) {
				itsDoExit = true;
			}
		}
	} // while

	if (!itsDelayedQuit) {
		stopHandlers();
	}
	else {
		itsDelayedQuit = false;	// never delay twice
		itsDoExit = false;		// we will run again
	}
}

//
// stopHandlers()
//
// An application can be stopped in two ways, a task itself may call stop() or
// the user may kill the program.
//
void GCFScheduler::stopHandlers()
{
	for (HandlerMap_t::iterator iter = itsHandlers.begin() ; iter != itsHandlers.end() ; ++iter) {
		if (iter->second)  {	// if pointer is still valid
			iter->first->stop();	// give handler a way of stopping in a neat way
		}
	} 

	// STEP 2
	LOG_INFO("Process is stopped! Possible reasons: 'stop' called or terminated");
	GCFHandler* pHandler(0);
	for (HandlerMap_t::iterator iter = itsHandlers.begin() ; iter != itsHandlers.end(); ++iter) {
		if (!iter->second)  {
			continue; // handler pointer is not valid anymore, 
		}
		// because this handler was deleted by the user
		pHandler = iter->first;
		pHandler->leave(); // "process" is also a handler user, see registerHandler
		if (pHandler->mayDeleted())  { // no other object uses this handler anymore?
			delete pHandler;
		}
	} 

	itsHandlers.clear();
}

//
// registerHandler(handler)
//
// NOTE: There are only a few handlers defined, timerHandler, fileHandler, PVSS Handler.
//	Those handlers handle the work of all 'ports' of that type. E.g. the timerHandler handles
// 	the decrement of all timers defined in all tasks!!!!
//
void GCFScheduler::registerHandler(GCFHandler& handler)
{
	LOG_TRACE_OBJ("GCFScheduler::registerHandler");
	itsHandlers[&handler] = true; // valid pointer
	handler.use(); // released after stop
}

//
// deregisterHandler(handler)
//
void GCFScheduler::deregisterHandler(GCFHandler& handler)
{
	LOG_TRACE_OBJ("GCFScheduler::deregisterHandler");
	HandlerMap_t::iterator iter = itsTempHandlers.find(&handler);
	if (iter != itsTempHandlers.end()) {
		iter->second = false; 	// pointer will be made invalid because the user 
								// deletes the handler by itself                          
	}
	itsHandlers.erase(&handler);
}

//
// _sendEvent
//
GCFEvent::TResult GCFScheduler::_sendEvent(GCFFsm* task, GCFEvent& event, GCFPortInterface*    port)
{
	if (task) {
		LOG_TRACE_CALC_STR("_sendEvent " << eventName(event) << "->task.doEvent");
		return (task->doEvent(event, port ? *port : *itsFrameworkPort));
	}
	LOG_TRACE_CALC_STR("_sendEvent " << eventName(event) << "->port.dispatch");
	return(port->dispatch(event));
}

//
// queueTransition
//
void GCFScheduler::queueTransition(GCFFsm* task, State	target, const char* from, const char* to)
{
	LOG_TRACE_CALC_STR("queueTransition from " << from << " to " << to);
	if (!itsUseQueue) {
		// when the queues are not used send them directly (in the right order).
		GCFEvent	exitEvent(F_EXIT);
		_sendEvent(task, exitEvent, 0);

		LOG_DEBUG(LOFAR::formatString ( "State transition to %s <<== %s", to, from));
		GCFFsm::GCFTranEvent	tranEvent;
		tranEvent.state = target;
		_sendEvent(task, tranEvent, 0);

		GCFEvent	entryEvent(F_ENTRY);
		_sendEvent(task, entryEvent, 0);
		return;
	}

	GCFEvent	entryEvent(F_ENTRY);
	_injectEvent(task, entryEvent, 0);

	LOG_DEBUG(LOFAR::formatString ( "State transition to %s <<== %s", to, from));
	GCFFsm::GCFTranEvent	tranEvent;
	tranEvent.state = target;
	_injectEvent(task, tranEvent, 0);

	GCFEvent	exitEvent(F_EXIT);
	_injectEvent(task, exitEvent, 0);
}


//
// queueEvent
//
void GCFScheduler::queueEvent(GCFFsm* task, GCFEvent& event, GCFPortInterface*    port)
{
	LOG_TRACE_CALC_STR("queueEvent " << eventName(event));
	LOG_TRACE_CALC_STR(event);
	if (!itsUseQueue) {
		_sendEvent(task, event, port);
		return;
	}

	// Framework events are always queued,
	if (F_EVT_PROTOCOL(event) == F_FSM_PROTOCOL || F_EVT_PROTOCOL(event) == F_PORT_PROTOCOL) {
		// prevent double entries of DATAIN and DISCONNECTED event of the same port.
		// (since the eventqueue is handled in pieces, the workProcs might be called > once before an event is processed).
		if ((event.signal == F_DATAIN || event.signal == F_DISCONNECTED) && _isInEventQueue(&event, port)) {
			return;
		}
		_addEvent(task, event, port);
		return;
	}

	// Events not generated by the framework are send immediately to the task to avoid copying.
	GCFEvent::TResult	status;
	if ((status = _sendEvent(task, event, port)) == GCFEvent::HANDLED) {
		return;	// its handled, we saved ourselves a copy.
	}

	switch (status) {
	case GCFEvent::NOT_HANDLED:
		LOG_TRACE_COND_STR("IGNORING event " << eventName(event) << " because return status is NOT_HANDLED");
		break;

	case GCFEvent::NEXT_STATE:
		LOG_DEBUG_STR("Moving event " << eventName(event) << " to eventQ of task, waiting there for state switch");
		task->queueTaskEvent(event, *port);
		break;

	case GCFEvent::HANDLED:
		break;
	} // switch
}

//
// _addEvent(task, event, port)
//
// Should only be called direct after an (external) event was received at a port.
//
void GCFScheduler::_addEvent(GCFFsm*			task, GCFEvent&			event, 
							 GCFPortInterface*	port)
{
	LOG_TRACE_CALC_STR("_addEvent " << eventName(event));
	waitingEvent_t*	newWE = new waitingEvent_t;
	newWE->task = task;
	newWE->port = (port ? port : itsFrameworkPort);
	newWE->seqNr = 0;
	newWE->event = event.clone();

	string	taskName((task ? ((GCFTask*)task)->getName() : "?"));
	LOG_TRACE_STAT_STR("theEventQueue.push(" << eventName(event) << "@" << newWE->port->getName() << 
				  " for " << taskName << ") => " << theEventQueue.size() + 1);

	theEventQueue.push_back(newWE);
}

//
// _injectEvent(task, event, port)
//
// Injects an event in FRONT of the queue
//
void GCFScheduler::_injectEvent(GCFFsm*				task, GCFEvent&		event, 
							 	GCFPortInterface*	port, bool			deepCopy)
{
	LOG_TRACE_CALC_STR("_injectEvent " << eventName(event));
	waitingEvent_t*	newWE = new waitingEvent_t;
	newWE->task = task;
	newWE->port = (port ? port : itsFrameworkPort);
	newWE->seqNr = 0;
	newWE->event = deepCopy ? event.clone() : &event;

	string	taskName((task ? ((GCFTask*)task)->getName() : "?"));
	LOG_TRACE_STAT_STR("theEventQueue.inject(" << eventName(event) << "@" << newWE->port->getName() << 
				  " for " << taskName << ") => " << theEventQueue.size() + 1);

	theEventQueue.push_front(newWE);
}
//
// handleEventQueue
//
void GCFScheduler::handleEventQueue()
{
	if (!itsUseQueue) {
		return;
	}

	printEventQueue();

	int	events2Handle = MIN2(BATCH_SIZE,theEventQueue.size()); // only handle the event that are in the queue NOW.
	while(events2Handle > 0) {
		waitingEvent_t*		theQueueEntry = theEventQueue.front();

		string	taskName((theQueueEntry->task ? ((GCFTask*)theQueueEntry->task)->getName() : "?"));
		LOG_TRACE_STAT_STR("theEventQueue.pop(" << eventName(*(theQueueEntry->event)) << "@" << 
					   theQueueEntry->port->getName() << " for " << taskName << 
					   ") <= " << theEventQueue.size());

		// remove the event from the Q so that transitions can be put in front of it.
		theEventQueue.pop_front();

		// pass it to the task
		GCFEvent::TResult	status = _sendEvent(theQueueEntry->task, *(theQueueEntry->event), 
												theQueueEntry->port);
	
		// Handle message according to return status.
		bool	handled(true);
		if (F_EVT_PROTOCOL(*(theQueueEntry->event)) == F_FSM_PROTOCOL) {
			// FSM events are never send again, you get one shot only.
			if (status != GCFEvent::HANDLED) {
				LOG_TRACE_COND_STR("Event " << eventName(*(theQueueEntry->event)) << " in task " << taskName << 
							 " NOT HANDLED, deleting it");
			}
			// when this command was an entry in a new state, inject the task queue into the current queue
			if (theQueueEntry->event->signal == F_ENTRY) {
				GCFFsm*				task(theQueueEntry->task);
				GCFEvent*			eventPtr;
				GCFPortInterface*	portPtr;
				while (task->unqueueTaskEvent(&eventPtr, &portPtr)) {
					LOG_DEBUG_STR("Injecting deferred taskEvent " << eventName(*eventPtr) << "into the event queue");
					_injectEvent(task, *eventPtr, portPtr, false);	// false=don't copy event(it's already cloned)
				}
			}
		}
		else { // all other protocols
			switch (status) {
			case GCFEvent::HANDLED:
				break;

			case GCFEvent::NOT_HANDLED:
				LOG_TRACE_COND_STR("DELETING event " << eventName(*(theQueueEntry->event)) << 
							  " because return status is NOT_HANDLED");
				handled = true;
				break;

			case GCFEvent::NEXT_STATE:
				LOG_DEBUG_STR("Moving event " << eventName(*(theQueueEntry->event)) << 
									" to eventQ of task, waiting there for state switch");
				theQueueEntry->task->queueTaskEvent(*(theQueueEntry->event), *(theQueueEntry->port));
				handled = false;
				break;
			} // switch
		} // else

		// when we started (an injected) transition make sure we finish the transaction before returning
		// control to other workProcs.
		if (theQueueEntry->event->signal == F_EXIT) {
			events2Handle = MAX2(events2Handle, 3);
		}

		// release memory
		LOG_TRACE_STAT_STR("Event " << eventName(*(theQueueEntry->event)) << " in task " << taskName << 
							 " removed from queue");
		delete theQueueEntry->event;
		theQueueEntry->event = 0;
		delete theQueueEntry;
		
		// one less to go.
		events2Handle--;
	} // while
}

//
// printEventQueue()
//
void GCFScheduler::printEventQueue()
{
	list<waitingEvent_t*>::iterator		iter = theEventQueue.begin();
	list<waitingEvent_t*>::iterator		end  = theEventQueue.end();
	int		nr(1);	
	while (iter != end) {
		string	taskName((*iter)->task ? ((GCFTask*)((*iter)->task))->getName() : "?");
		LOG_TRACE_STAT_STR("theEventQueue[" << nr << "] = (" << eventName(*((*iter)->event)) << "@" << 
					   (*iter)->port->getName() << " for " << taskName << ")");
		nr++;
		++iter;
	}
}

//
// _isInEventQueue(GCFEvent*)
//
bool GCFScheduler::_isInEventQueue(GCFEvent*	someEvent, GCFPortInterface*	somePort)
{
	list<waitingEvent_t*>::iterator		iter = theEventQueue.begin();
	list<waitingEvent_t*>::iterator		end  = theEventQueue.end();
	while (iter != end) {
		if ((*iter)->event->signal == someEvent->signal && (*iter)->port == somePort) {
			LOG_DEBUG_STR("Event " << eventName(*((*iter)->event)) << "@" << (*iter)->port->getName() 
						<< " already in the queue!");
			return (true);
		}
		++iter;
	}
	return (false);
}

//
// signalHandler(sig)
//
void GCFScheduler::signalHandler(int sig)
{
	if ((sig == SIGINT) || (sig == SIGTERM)) {
		GCFScheduler::instance()->stop();
	}
}                                            

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
