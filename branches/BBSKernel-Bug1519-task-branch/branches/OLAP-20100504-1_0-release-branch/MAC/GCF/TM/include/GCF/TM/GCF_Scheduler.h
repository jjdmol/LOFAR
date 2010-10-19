//#  GCF_Scheduler.h: handles all events for a task.
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

#ifndef GCF_SCHEDULER_H
#define GCF_SCHEDULER_H

#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_PortInterface.h>

namespace LOFAR {
  using MACIO::GCFEvent;
  namespace GCF {
    namespace TM {

// forward declaration
class GCFPort;
class GCFHandler;
class GCFFsm;

// This type of port can be used to transport internal messages linked to a 
// port, which does not implement anything of the typical features of a port
class GCFDummyPort : public GCFPortInterface
{
public:
	GCFDummyPort (GCFTask* pTask, string name, int protocol) : 
		GCFPortInterface(pTask, name, SPP, protocol, false) 
	{}

	// Since we inherit from ABC GCFPortInterface we must 'implement' all functions.
	bool close () 								  { return (false); }
	bool open () 								  { return (false); }
	ssize_t send (GCFEvent&) 					  { return (0);	}
	ssize_t recv (void*, size_t) 				  { return (0);	}
	long setTimer (long, long, long, long, void*) {	return (0);	}
	long setTimer (double, double, void*)		  { return (0);	}
	int  cancelTimer (long, void**)				  { return (0);	}
	int  cancelAllTimers ()						  { return (0);	} 
	double timeLeft(long)						  { return (0);	}

private:
	// not allowed
	GCFDummyPort();
	GCFDummyPort(GCFDummyPort&);
    GCFDummyPort& operator= (GCFDummyPort&);
};

// This is the base class for all tasks in an application. Different 
// specialisations of this class results in a number of concurrent finite state 
// machines with own ports to other tasks (in other processes). 
// Note: This is not a representation of a 'thread' related to the 
// multithreading concept.
class GCFScheduler
{
public:  
	// Construction via singleton mechanism
	static GCFScheduler*	instance();
    
    // Inits a number of services for the GCF based application:
    // - holds the argc and argv parameters in static data members
    // - lofar logger(argv[0].log_prop or "mac.log_prop")
    // - parameterset(argv[0] + ".conf")
    void init (int argc, char** argv, const string&	logfile = "");
    
    // The run method. This starts the event processing loop.
    // When multiple tasks are declared within the same binary, only
    // one call to GCFScheduler::run will suffice.
    // A call to this function will NEVER return (except on stop). 
    //
    // @code
    //     class MyTask : public GCFTask { ... }
    //
    //     int main(int argc, char** argv)
    //     {
    //         // create two tasks, a and b.
	//         GCFScheduler*	theSched = GCFScheduler::instance();
    //         theSched.init(argc, argv);
    // 
    //         MyTask a("a");
    //         MyTask b("b");
    //         a.start();
    //         b.start();
    //         // start the event processing loop
    //         theSched->run();
    //     }
    // 
    // @endcode
    //
	// The method described above has one major drawback: When one task calls 
	// stop() the others tasks are also immediately stopped. They done get a 
	// change to stop their actions in a neat way.
	// In Nov. 2007 the F_QUIT signal was introduced that solves this problem.
	// When stop is called each task can receive a F_QUIT signal allowing the
	// task to do its closing actions.
	// The main loop has to be changed into:
	//
	// @code
    //     class MyTask : public GCFTask { ... }
    //
    //     int main(int argc, char** argv)
    //     {
    //         // create two tasks, a and b.
	//         GCFScheduler*	theSched = GCFScheduler::instance();
    //         theSched.init(argc, argv);
    // 
    //         MyTask a("a");
    //         MyTask b("b");
    //         a.start();
    //         b.start();
	//
	//		   theSched->setDelayedQuit(true);		// <--
    //         // start the event processing loop
    //         theSched->run();
	//
	//		   // when stop() was called we continue here
	//		   a.quit();			// send the F_QUIT signals
	//		   b.quit();
	//		   theSched->run(2);	// let it run for another 2 seconds
    //     }
    // 
    // @endcode
    void run (double	seconds = 0.0);
    
    // registers a GCFHandler for the mainloop
    void registerHandler (GCFHandler& handler);
    
    // deregisters a GCFHandler from the mainloop
    void deregisterHandler (GCFHandler& handler);

    // stops the application; it stops all registered handlers
    void stop () {  itsDoExit = true; }

	// specificy whether or not F_QUIT signals are send after stop
	// to allow the tasks to do their final run.
	void setDelayedQuit(bool	newState) { itsDelayedQuit = newState; }
 
	//  data members
    //	the command line arguments passed to the application at start-up
    //	they are set by means of the static init method
    //    
    static int _argc;
    static char** _argv;
   
	void queueEvent(GCFFsm*	task, GCFEvent&	event, GCFPortInterface* port);
	void printEventQueue();
	void disableQueue() { itsUseQueue = false; }

protected:
	// makes sure that the current task makes a transition to the new state.
    typedef GCFEvent::TResult (GCFFsm::*State)(GCFEvent& event, GCFPortInterface& port); // ptr to state handler type
	void queueTransition(GCFFsm*	task, State target, const char* from, const char* to);
	friend class GCFFsm;	// to use this queueTransition function.

private:
	void				_addEvent   (GCFFsm* task, GCFEvent& event, GCFPortInterface* port);
	void 				_injectEvent(GCFFsm* task, GCFEvent& event, GCFPortInterface* port, bool deepCopy=true);
	GCFEvent::TResult	_sendEvent  (GCFFsm* task, GCFEvent& event, GCFPortInterface* port);

    // Singleton
    GCFScheduler();
	GCFScheduler(const GCFScheduler&	that);
	GCFScheduler& operator=(const GCFScheduler& that);
	~GCFScheduler();

    // handles system signals
    static void signalHandler(int sig);

	// give a stop signal to all the handlers
	void stopHandlers();

	void handleEventQueue();

	// --- DATA MEMBERS ---
	// Structure for the eventqueue
	typedef struct {
		GCFEvent*			event;
		GCFPortInterface*	port;
		GCFFsm*				task;
		int					seqNr;
	} waitingEvent_t;
	list<waitingEvent_t*>	theEventQueue;

    // all registered handlers, which should be invoked (workProc) circulair
    typedef map<GCFHandler*, bool /*valid*/> HandlerMap_t;
    HandlerMap_t	itsHandlers;
    HandlerMap_t 	itsTempHandlers;

    /// all registered protocols in the application
    typedef map<unsigned short, const char**> ProtocolMap_t;
    ProtocolMap_t	itsProtocols;

    /// indicates wether the application should be stopped or not
    bool	itsDoExit;

	// Indicates if the tasks will stop immediately after the stop command or
	// if the will receive a final F_QUIT signal.
	bool	itsDelayedQuit;

	// whether or not Scheduler was initiated.
	bool	itsIsInitialized;

	bool	itsUseQueue;

	// Dummy port to be passed in event we make.
    GCFDummyPort*			itsFrameworkPort;

	static GCFScheduler*	theirGCFScheduler;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
