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

#include <GCF/TM/GCF_Event.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_PortInterface.h>

#include <iostream>
#include <cstdlib>

#define TRAN(_target_) \
  { \
    tran(static_cast<State>(&_target_), __func__, #_target_); \
  }

/**
 * This type of port can be used to transport internal messages linked to a 
 * port, which does not implement anything of the typical features of a port
 */
class GCFDummyPort : public GCFPortInterface
{
  public:
    GCFDummyPort (GCFTask* pTask, 
                  string name, 
                  int protocol) : 
      GCFPortInterface(pTask, name, SPP, protocol, false) 
    {};

    inline int close () {return 0;}
    inline int open () {return 0;}

    inline ssize_t send (GCFEvent& /*event*/)
    {
      return 0;
    }
    

    inline ssize_t recv (void* /*buf*/, 
                         size_t /*count*/) 
    {
      return 0;
    }

    inline long setTimer (long  /*delay_sec*/,
                          long  /*delay_usec*/,
                          long  /*interval_sec*/,
                          long  /*interval_usec*/,
                          void* /*arg*/) 
    {
      return 0;
    }

    inline long setTimer (double /*delay_seconds*/, 
                          double /*interval_seconds*/,
                          void*  /*arg*/)
    {
      return 0;
    }

    inline int cancelTimer (long /*timerid*/, 
                            void** /*arg*/) 
    {
      return 0;
    }

    inline int cancelAllTimers () {return 0;}

    inline int resetTimerInterval (long /*timerid*/,
                                   long /*sec*/,
                                   long /*usec*/) 
    {
      return 0;
    }
};

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
    typedef GCFEvent::TResult (GCFFsm::*State)(GCFEvent& event, GCFPortInterface& port); // ptr to state handler type
    
    GCFFsm (State initial) : _state(initial) {;} 
    virtual ~GCFFsm () {;}
  
    void initFsm ();
  
    inline GCFEvent::TResult dispatch (GCFEvent& event, 
                                       GCFPortInterface& port)
    {
      return (this->*_state)(event, port);
    }
  
  protected:
  
    void tran (State target, 
               const char* from, 
               const char* to);
  
  protected:
    volatile State _state;
  
  private:
    GCFFsm();
    static GCFDummyPort _gcfPort;
};


#endif
