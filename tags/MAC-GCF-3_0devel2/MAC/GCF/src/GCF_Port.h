//#  GCF_Port.h: represents a protocol port to a remote process
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

#ifndef GCF_PORT_H
#define GCF_PORT_H

#include <GCF/GCF_PortInterface.h>
#include <GCF/GCF_PeerAddr.h>

// forward declaration
class GCFTask;
class GCFRawPort;

/**
 * This class represents a protocol port that is used to exchange events defined 
 * in a protocol independent from the transport mechanism. On initialisation of 
 * the port it creates a concrete raw port class (with the transport mechanism). 
 * It is possible to init GCFPort's with complete information or the information 
 * can be received from a name service. This port is always the master of raw 
 * ports.
 */
class GCFPort : public GCFPortInterface
{
  public:

    /**
    * constructors
    * @param protocol NOT USED
    */
    GCFPort (GCFTask& containertask, 
             string& name, 
             TPortType type, 
             int protocol, 
             bool transportRawData = false);
    
    GCFPort ();
    
    /**
    * destructor
    */
    virtual ~GCFPort ();
    
    /** initialize function, to follow-up default constructor */
    void init (GCFTask& containertask, 
               string name, 
               TPortType type, 
               int protocol, 
               bool transportRawData = false);
    
    /**
    * open/close functions
    */
    virtual int open ();
    virtual int close ();
    
    /**
    * send/recv functions
    */
    virtual ssize_t send (const GCFEvent& event, 
                          void* buf = 0, 
                          size_t count = 0);                          
    virtual ssize_t sendv (const GCFEvent& event,
                           const iovec buffers[], 
                           int n);
    virtual ssize_t recv (void* buf, 
                          size_t count);
    virtual ssize_t recvv (iovec buffers[], 
                           int n);
    
    /**
    * Timer functions.
    * Upon expiration of a timer a F_TIMER_SIG will be
    * received on the port.
    */    
    virtual long setTimer (long  delay_sec,         long  delay_usec    = 0,
                           long  interval_sec  = 0, long  interval_usec = 0,
                           void* arg     = 0);
    
    virtual long setTimer (double delay_seconds, 
                           double interval_seconds = 0.0,
                           void*  arg        = 0);
    
    virtual int  cancelTimer (long  timerid,
                              void** arg = 0);
    
    virtual int  cancelAllTimers ();
    
    /**
     * THIS METHOD IS NOT IMPLEMENTED YET
     */
    virtual int  resetTimerInterval (long timerid,
                                     long sec,
                                     long usec = 0);

  private:

    /**
    * Don't allow copying this object.
    */
    GCFPort (const GCFPort&);
    GCFPort& operator= (const GCFPort&);

    friend class GCFRawPort;
 
  private:

    GCFPeerAddr     _localAddr;
    GCFPeerAddr     _remoteAddr;
    
    GCFPortInterface* _pSlave;
};
#endif
