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

//# Includes
#include <TM/GCF_Fsm.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>

// forward declaration
class GCFPort;
class GCFHandler;

/**
 * Base class of all application tasks. This class inherits from GCFFsm through
 * which behaviour of the task in terms of states, events and transitions is
 * defined.
 *
 */

class GCFTask : public GCFFsm
{
  public:

    /**
    * The static run method. This starts the event processing loop.
    * When multiple tasks are declared within the same binary, only
    * one call to GCFTask::run will suffice.
    * A call to this function will NEVER return. It is an error if it
    * does return.
    *
    * @code
    *
    *     class MyTask : public GCFTask { ... }
    *
    *     int main(int,char**)
    *     {
    *         // create two tasks, a and b.
    *         MyTask a("a");
    *         MyTask b("b");
    *		      a.start();
    * 		    b.start();
    *         // start the event processing loop
    *         GCFTask::run();
    *     }
    * 
    * @endcode
    */
    void start();
    static void run();
    static void registerHandler(GCFHandler& handler);
    static void stop();
 
    // Get the name of the task.
    inline string& getName() {return _name;}
    /// Set the name of the task.
    inline void setName(string& name) {_name = name;}
    static int _argc;
    static char** _argv;
   
  protected:
		GCFTask(State initial, string& name); 
		virtual ~GCFTask();
    /**
    * Register the protocol. This is used for debugging. The name of each event
    * that is part of the protocol is specified. Index 0 should not be used for
    * a signal. Signal numbers should start at 1.
    */
    void registerProtocol(unsigned short protocol_id,
                                        const char* signal_names[]);
    friend class GCFPort;
    const char* evtstr(const GCFEvent& e);

  private:
		/// Is private to avoid initialising a task without giving an inital state and the task name
    GCFTask();
    string _name;
    static vector<GCFHandler*> _handlers;
    typedef vector<GCFHandler*>::iterator THandlerIter;

    static map<unsigned short, const char**> _protocols;
    typedef map<unsigned short, const char**>::iterator TProtocolIter;

    static void signalHandler(int sig);
    static bool _doExit;
};
#endif
