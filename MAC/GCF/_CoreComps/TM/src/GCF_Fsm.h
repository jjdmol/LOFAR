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

#include <TM/GCF_Event.h>
#include <TM/GCF_TMProtocols.h>
#include <TM/PortInterface/GCF_PortInterface.h>
#include <iostream>
#include <cstdlib>

#define TRAN(_target_) tran(static_cast<State>(_target_))

/**
 * Simple Optimal FSM implementation from 
 * "Practical Statecharts in C/C++", Samek, M., CMP Books, 2002.
 *
 * @todo * Implement the <i>hierarchical</i> state machines.
 */

class GCFDummyPort : public GCFPortInterface
{
  public:
    GCFDummyPort(GCFTask* pTask, string name, int protocol) : 
      GCFPortInterface(pTask, name, SPP, protocol) {};

    inline int close() {return 0;}
    inline int open() {return 0;}

    inline ssize_t send(const GCFEvent& /*event*/, 
                        void* /*buf*/ = 0, 
                        size_t /*count*/ = 0)
    {
      return 0;
    }
    
    inline ssize_t sendv(const GCFEvent& /*event*/, 
                  const iovec /*buffers*/[], int /*n*/) 
    {
      return 0;
    }

    inline ssize_t recv(void* /*buf*/, size_t /*count*/) 
    {
      return 0;
    }

    inline ssize_t recvv(iovec /*buffers*/[], int /*n*/) 
    {

      return 0;
    }

    inline long setTimer(
        long  /*delay_sec*/,
        long  /*delay_usec*/,
        long  /*interval_sec*/,
        long  /*interval_usec*/,
        const void* /*arg*/) 
    {
      return 0;
    }

    inline long setTimer(
        double /*delay_seconds*/, 
        double /*interval_seconds*/,
        const void*  /*arg*/)
    {
      return 0;
    }

    inline int cancelTimer(long /*timerid*/, const void** /*arg*/) 
    {
      return 0;
    }

    inline int cancelAllTimers() {return 0;}

    inline int resetTimerInterval(
        long /*timerid*/,
        long /*sec*/,
        long /*usec*/) 
    {
      return 0;
    }
};

class GCFFsm
{
  public:
    typedef int (GCFFsm::*State)(GCFEvent& event, GCFPortInterface& port); // ptr to state handler type
    
    GCFFsm(State initial) : _state(initial) {;} 
    virtual ~GCFFsm() {;}
  
    void initFsm()
    {
      GCFEvent e;
      e.signal = F_ENTRY_SIG;
      (void)(this->*_state)(e, _gcfPort); // entry signal
      e.signal = F_INIT_SIG;
      if (0 != (this->*_state)(e, _gcfPort)) // initial transition
      {
        cerr << "Fsm::init: initial transition F_SIGNAL(F_FSM_PROTOCOL, F_INIT_SIG) not handled." << endl;
        exit(1); // EXIT
      }
    }
  
    inline int dispatch(GCFEvent& event, GCFPortInterface& port)
    {
      return (this->*_state)(event, port);
    }
  
  protected:
  
    inline void tran(State target)
    {
      GCFEvent e;
      e.signal = F_EXIT_SIG;
      (void)(this->*_state)(e, _gcfPort); // exit signal
    
      _state = target; // state transition
      
      e.signal = F_ENTRY_SIG;
      (void)(this->*_state)(e, _gcfPort); // entry signal
    }
  
  protected:
    volatile State _state;
  
  private:
    static GCFDummyPort _gcfPort;
};


#endif
