//#  GCF_Task.h: handles all events for a task.
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

#ifndef GCF_TASK_H
#define GCF_TASK_H

#include <Common/lofar_map.h>
#include <GCF/TM/GCF_Fsm.h>

namespace LOFAR {
  namespace GCF {
    namespace TM {

// forward declaration
class GCFPort;
class GCFHandler;

/**
 * This is the base class for all tasks in an application. Different 
 * specialisations of this class results in a number of concurrent finite state 
 * machines with own ports to other tasks (in other processes). 
 * Note: This is not a representation of a 'thread' related to the 
 * multithreading concept.
 */

class GCFTask : public GCFFsm
{
public:  
	// constuctors
    virtual ~GCFTask();

    // "starts" this task; see code example from run method
    void start () { initFsm(); }

	// Gives task a final change to end their actions
	void quit () { quitFsm();	}
    
    // -- static method --
    // Inits a number of services for the GCF based application:
    // - holds the argc and argv parameters in static data members
    // - lofar logger(argv[0].log_prop or "mac.log_prop")
    // - parameterset(argv[0] + ".conf")
    //     
    static void init (int argc, char** argv, const string&	logfile = "");
    
    // The static run method. This starts the event processing loop.
    // When multiple tasks are declared within the same binary, only
    // one call to GCFTask::run will suffice.
    // A call to this function will NEVER return (except on stop). 
    //
    // @code
    //     class MyTask : public GCFTask { ... }
    //
    //     int main(int argc, char** argv)
    //     {
    //         // create two tasks, a and b.
    //         GCFTask::init(argc, argv);
    // 
    //         MyTask a("a");
    //         MyTask b("b");
    //         a.start();
    //         b.start();
    //         // start the event processing loop
    //         GCFTask::run();
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
    //         GCFTask::init(argc, argv);
    // 
    //         MyTask a("a");
    //         MyTask b("b");
    //         a.start();
    //         b.start();
	//		   GCFTask::setDelayedQuit(true);		// <--
	//
    //         // start the event processing loop
    //         GCFTask::run();
	//
	//		   // when stop() was called we continue here
	//		   a.quit();		// send the F_QUIT signals
	//		   b.quit();
	//		   GCFTask::run(2);	// let it run for another 2 seconds
    //     }
    // 
    // @endcode


    static void run (double	seconds = 0.0);
    
    // registers a GCFHandler for the mainloop
    static void registerHandler (GCFHandler& handler);
    
    // deregisters a GCFHandler from the mainloop
    static void deregisterHandler (GCFHandler& handler);

    // stops the application; it stops all registered handlers
    static void stop () {  _doExit = true; }

	// specificy whether or not F_QUIT signals are send after stop
	// to allow the tasks to do their final run.
	static void setDelayedQuit(bool	newState) { _delayedQuit = newState; }
 
    // Get the name of the task.
    const string& getName () const {return _name;}
    // Set the name of the task.
    void setName (string& name) {_name = name;}

	//  data members
    //	the command line arguments passed to the application at start-up
    //	they are set by means of the static init method
    //    
    static int _argc;
    static char** _argv;
   
protected:
    explicit GCFTask (State initial, 
                      const string& name); 

private:
    friend class GCFPort;
    friend class GCFRawPort;

    // Is private to avoid initialising a task without giving an inital state and the task name
    GCFTask();
  
    // handles system signals
    static void signalHandler(int sig);

	// give a stop signal to all the handlers
	static void stopHandlers();

	// --- DATA MEMBERS ---
    // the task name
    string _name;
    // all registered handlers, which should be invoked (workProc) circulair
    typedef map<GCFHandler*, bool /*valid*/> THandlers;
    static THandlers _handlers;
    static THandlers _tempHandlers;

    /// all registered protocols in the application
    typedef map<unsigned short, const char**> TProtocols;
    static TProtocols _protocols;

    /// indicates wether the application should be stopped or not
    static bool _doExit;

	// Indicates if the tasks will stop immediately after the stop command or
	// if the will receive a final F_QUIT signal.
	static bool _delayedQuit;

};  
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
