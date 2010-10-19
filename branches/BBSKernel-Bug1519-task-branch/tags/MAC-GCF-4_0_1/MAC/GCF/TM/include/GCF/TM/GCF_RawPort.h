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

#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_PeerAddr.h>
#include <GCF/TM/GCF_Event.h>
#include <GCF/TM/GCF_Task.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>

// forward declaration
class GCFPort;
class GTMTimer;
class GTMTimerHandler;

/**
 * This is the abstract base class for all concrete port implementations (like 
 * TCP, shared memory). A concrete raw port can be a slave of the GCFPort class 
 * or a master of itself. If it is a slave than the GCFPort has created the 
 * concrete raw port to hide the transport mechanism. Otherwise the concrete raw 
 * port is deliberate instantiated by a concrete task.
 */
class GCFRawPort : public GCFPortInterface
{
  public:
    /** @param protocol NOT USED */
    explicit GCFRawPort (GCFTask& task, 
                string& name, 
                TPortType type, 
                int protocol, 
                bool transportRawData = false);
  
    explicit GCFRawPort ();
  
    virtual ~GCFRawPort ();
  
    /** @param protocol NOT USED */
    void init (GCFTask& task, 
               string name, 
               TPortType type, 
               int protocol, 
               bool transportRawData = false); 
  
    /// GCFPortInterface methods
  
     /**
     * Timer functions.
     * Upon expiration of a timer a F_TIMER_SIG will be
     * received on the port.
     */
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
  
    virtual int  resetTimerInterval (long timerid,
                          				   long sec,
                          				   long usec = 0);


  protected:
    friend class GCFPort; // to access the setMaster method
    friend class GTMTimer;
    friend class GTMSocket;
    friend class GTMTCPServerSocket;

    void schedule_disconnected();
    void schedule_close();
    void schedule_connected();

    inline bool                 isSlave () const {return _pMaster != 0;}
    virtual void                setMaster (GCFPort* pMaster);
    virtual GCFEvent::TResult   dispatch (GCFEvent& event);
    bool                        findAddr (GCFPeerAddr& addr);

  private:

    /**
    * Don't allow copying this object.
    */   
    GCFRawPort (const GCFRawPort&);
    GCFRawPort& operator= (const GCFRawPort&);

    GCFPort* _pMaster;

    GCFEvent::TResult recvEvent();
    GTMTimerHandler*  _pTimerHandler;
};

#endif
