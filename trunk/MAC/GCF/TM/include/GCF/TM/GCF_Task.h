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

#include <GCF/TM/GCF_Fsm.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

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
  
  public:  // constuctors && destructors
    virtual ~GCFTask();

  protected:
    explicit GCFTask (State initial, 
                      const string& name); 
  private:
    /// Is private to avoid initialising a task without giving an inital state and the task name
    GCFTask();
  
  public: // member methods

    /// "starts" this task; see code example from run method
    void start () { initFsm(); }
    
    /** static method; 
     * inits a number of services for the GCF based application:
     * - holds the argc and argv parameters in static data members
     * - lofar logger("mac.log_prop")
     * - parameterset(argv[0] + ".conf")
     */     
    static void init (int argc, char** argv);
    
    /**
    * The static run method. This starts the event processing loop.
    * When multiple tasks are declared within the same binary, only
    * one call to GCFTask::run will suffice.
    * A call to this function will NEVER return (except on stop). 
    *
    * @code
    *
    *     class MyTask : public GCFTask { ... }
    *
    *     int main(int argc, char** argv)
    *     {
    *         // create two tasks, a and b.
    *         GCFTask::init(argc, argv);
    * 
    *         MyTask a("a");
    *         MyTask b("b");
    *         a.start();
    *         b.start();
    *         // start the event processing loop
    *         GCFTask::run();
    *     }
    * 
    * @endcode
    */
    static void run ();
    
    /// registers a GCFHandler for the mainloop
    static void registerHandler (GCFHandler& handler);
    
    /// stops the application; it stops all registered handlers
    static void stop ();
 
    // Get the name of the task.
    const string& getName () const {return _name;}
    /// Set the name of the task.
    void setName (string& name) {_name = name;}

    /// returns the "define" of a signal ID as a string    
    const char* evtstr(const GCFEvent& e) const;

  protected:
    /**
    * Register the protocol. This is used for logging. The name of each event
    * that is part of the protocol is specified. Index 0 should not be used for
    * a signal. Signal numbers should start at 1.
    */
    void registerProtocol(unsigned short protocol_id,
                          const char* signal_names[]);

  private:
    /// handles system signals
    static void signalHandler(int sig);

  public: // data members
    /** the command line arguments passed to the application at start-up
     * they are set by means of the static init method
     */    
    static int _argc;
    static char** _argv;
   
  private:
    friend class GCFPort;
    friend class GCFRawPort;
    /// the task name
    string _name;
    /// all registered handlers, which should be invoked (workProc) circulair
    typedef vector<GCFHandler*> THandlers;
    static THandlers _handlers;

    /// all registered protocols in the application
    typedef map<unsigned short, const char**> TProtocols;
    static TProtocols _protocols;

    /// indicates wether the application should be stop or not
    static bool _doExit;
};  
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
