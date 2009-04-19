//#  GCF_Fsm.h: header file for the finite state machine implementation.
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

#ifndef GCF_FSM_H
#define GCF_FSM_H

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Scheduler.h>

namespace LOFAR {
  using MACIO::GCFEvent;
  namespace GCF {
	namespace TM {

#define TRAN(_target_) \
  { \
    tran(this, static_cast<State>(&_target_), __func__, #_target_); \
  }

/**
 * Fsm = Finite State Machine
 * All tasks implement their behaviour in terms of a finite state machine. The 
 * implementation in the GCFFsm class from which the GCFTask derives is a direct 
 * implementation of the design described in [ref. AD.6]. This design is based 
 * around the concept of state event handlers. Events are received when the 
 * state machine is in a certain state. This state is represented by a method of 
 * the APLExampleTaskA class (derived from GCFFTask). This state handler method 
 * is called when a new event arrives for the task. The handler has two 
 * arguments: a reference to the event (GCFEvent) that was received, and a 
 * reference to the port on which it was received (GCFPortInterface).
 * @code

       int my_state(GCFEvent& e, GCFPortInterface& p);
       
 * @encode
 * A task is therefore an event driven application. In fact the programmer using 
 * the sub-framework does not control the main loop of the application. This 
 * loop is handled by the framework through the GCFTask::run() method.
 */
class GCFFsm
{
public:
	// ----- Internal type definitions -----
	// Define the State function_type.
    typedef GCFEvent::TResult (GCFFsm::*State)(GCFEvent& event, GCFPortInterface& port); // ptr to state handler type
    
    /// let the current State from the adapted task execute the event.
    GCFEvent::TResult doEvent (GCFEvent& 		 event, 
							   GCFPortInterface& port)
    { 
		if (event.signal == F_TRAN) {
			GCFTranEvent*	tranEvent((GCFTranEvent*)&event);
			itsState = tranEvent->state;
			return (GCFEvent::HANDLED);
		}
		return (this->*itsState)(event, port); 
	}

protected: // constructors && destructors
	// Define TRANEvent
	struct GCFTranEvent : public GCFEvent
	{
		GCFTranEvent() : GCFEvent(F_TRAN)
		{
			length = sizeof(GCFTranEvent);
		}
		State 	state;
	};

    explicit  GCFFsm (State initial) : itsState(initial) {
		itsScheduler = GCFScheduler::instance();
	}

    virtual ~GCFFsm () {;}
  
    /// starts the statemachine with a F_ENTRY signal followed by a F_INIT signal
    void initFsm ();
  
	/// send F_QUIt signal
	void quitFsm();
  
    // state transition; will be used by the MACRO TRAN see above
    // sends a F_EXIT signal to the current state followed by the state transition
    // and finaly sends a F_ENTRY signal to the new current state
    // @param target new state
    // @param from text of the current state
    // @param to text of the new state
    void tran (GCFFsm*	task,
			   State target, 
               const char* from, 
               const char* to);
  
	// Allow the scheduler to manipulate my eventQueue.
	friend class GCFScheduler;
	void queueTaskEvent(GCFEvent&	event, GCFPortInterface&	port);
	void handleTaskQueue();

private:
	// Default construction and copying is not allowed.
    GCFFsm();
	GCFFsm(const GCFFsm& that);
	GCFFsm& operator=(const GCFFsm& that);
	
	// ----- DATA MEMBERS -----
	// Structure for the eventqueue
	typedef struct {
		GCFEvent*			event;
		GCFPortInterface*	port;
	} waitingTaskEvent_t;
	list<waitingTaskEvent_t*>	itsEventQueue;

    State 			itsState;
	GCFScheduler*	itsScheduler;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
