//#  GCF_RawPort.h: connection to a remote process
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

#ifndef GCF_RAWPORT_H
#define GCF_RAWPORT_H

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Scheduler.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

namespace LOFAR {
 using MACIO::GCFEvent;
 namespace GCF {
  namespace TM {

// forward declaration
class GCFPort;
class GTMTimer;
class GTMTimerHandler;

typedef struct
{
  string taskname;
  string portname;
} TPeerAddr;

/**
 * This is the abstract base class for all concrete port implementations (like 
 * TCP, shared memory). A concrete raw port can be a slave of the GCFPort class 
 * or a master of itself. If it is a slave than the GCFPort has created the 
 * concrete raw port to hide the transport mechanism. Otherwise the concrete raw 
 * port is deliberate instantiated by a concrete task. Its main responsibilities are:
 * - makes the timer functionality available
 * - convert incomming data to GCFEvent format (if transportRawData is false) and
 *   dispatches the event to the adapted task
 * - dispatches framework events (like F_CONNECTED)
 */
class GCFRawPort : public GCFPortInterface
{
	// NOTE: constructors are protected !!!

public:
    /// desctructor
    virtual ~GCFRawPort ();
  
    /// params see constructor of GCFPortInterface
    void init (GCFTask& 	 task, 
               const string& name, 
               TPortType 	 type, 
               int 			 protocol, 
               bool 		 transportRawData = false); 

	// GCFPortInterface overloaded/defined methods
  
    /// these methods implements the final connection with the timer handler 
    /// to realize the timer functionality
    virtual long setTimer (long  delay_sec,
                           long  delay_usec    = 0,
                           long  interval_sec  = 0,
                           long  interval_usec = 0,
                           void* arg     = 0);
  
    virtual long setTimer (double delay_seconds, 
                    			 double interval_seconds = 0.0,
                    			 void*  arg        = 0);
  
    virtual int  cancelTimer (long timerid,
  			                      void** arg = 0);
  
    virtual int  cancelAllTimers ();

	virtual double	timeLeft(long	timerID);

    virtual GCFEvent::TResult   dispatch(GCFEvent& event);
protected: 
	// constructors && destructors
    /// params see constructor of GCFPortInterface
    GCFRawPort (GCFTask& 		task, 
						 const string& 	name, 
						 TPortType 		type, 
						 int 			protocol, 
						 bool 			transportRawData = false);

    /** default constructor 
     * GCFPortInterface params are:
     * pTask => 0
     * name => ""
     * type => SAP
     * protocol => 0
     * transportRawData => false
     */ 
    GCFRawPort();
    
	// helper methods
    friend class GCFPort; // to access the setMaster method
//    friend class GTMTimer;//
//    friend class GTMFile; // they all call dispatch
//    friend class GCFTask; //

    void schedule_disconnected() {
		GCFEvent	event(F_DISCONNECTED);
		GCFScheduler::instance()->queueEvent(0, event, this);
	}
    void schedule_close() {
// IN SOME CASES THE PORT IS DELETED IMMEDIATELY AFTER THE CLOSE, WHEN THE DELAYED
// EVENT IS HANDLED AFTERWARDS THE PORT VALUE IS INVALID --> Segmentation fault.
// TRY IT WITHOUT THE CLOSE EVENTS. WHO IT USING THOSE?
//		GCFEvent	event(F_CLOSED);
//		GCFScheduler::instance()->queueEvent(0, event, this);
	}
    void schedule_connected() {
		GCFEvent	event(F_CONNECTED);
		GCFScheduler::instance()->queueEvent(0, event, this);
	}

    bool                        isSlave () const {return _pMaster != 0;}
    virtual void                setMaster (GCFPort* pMaster) {_pMaster = pMaster;}

    GCFEvent::TResult           recvEvent();

    bool                        findAddr (TPeerAddr& addr);
    
    /// returns the original name of the port (given by the user). 
    /// in case it is a slave port an extension is append to the original name 
    string		                getRealName() const;  

	// admin. data member
    GTMTimerHandler*            _pTimerHandler;

private: 
    /// copying is not allowed.
    GCFRawPort (const GCFRawPort&);
    GCFRawPort& operator= (const GCFRawPort&);

	// data member
    GCFPort* 					_pMaster;

	// Pointer to scheduler
	GCFScheduler*				itsScheduler;
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
