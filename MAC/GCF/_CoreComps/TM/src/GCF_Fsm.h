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
#include <TM/GTM_Defines.h>
#include <TM/GCF_TMProtocols.h>
#include <TM/PortInterface/GCF_PortInterface.h>
#include <iostream>
#include <cstdlib>

#define TRAN(_target_) \
  { \
    LOFAR_LOG_TRACE(TM_STDOUT_LOGGER, (\
        "State transition to %s <<== %s", \
        #_target_, \
        __func__)); \
    tran(static_cast<State>(&_target_)); \
  }


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
    typedef GCFEvent::TResult (GCFFsm::*State)(GCFEvent& event, GCFPortInterface& port); // ptr to state handler type
    
    GCFFsm(State initial) : _state(initial) {;} 
    virtual ~GCFFsm() {;}
  
    void initFsm();
  
    inline GCFEvent::TResult dispatch(GCFEvent& event, GCFPortInterface& port)
    {
      return (this->*_state)(event, port);
    }
  
  protected:
  
    void tran(State target);
  
  protected:
    volatile State _state;
  
  private:
    static GCFDummyPort _gcfPort;
};


#endif
