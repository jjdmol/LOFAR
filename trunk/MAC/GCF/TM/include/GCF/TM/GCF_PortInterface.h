//#  GCF_PortInterface.h: container class for all port implementations
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

#ifndef GCF_PORTINTERFACE_H
#define GCF_PORTINTERFACE_H

#include <sys/types.h>
#include <sys/uio.h>
#include <string.h>

#include <Common/lofar_string.h>

// forward declacations
class GCFTask;
class GCFEvent;

/**
 * This is the abstract base class for all port implementations like TCP, shared 
 * memory. It provides the possibility to:
 * - send and receive events to/from peers in a generic way.
 * - start and stop timers on the port
 */
class GCFPortInterface
{
  public:

    /**
    * port types
    */
    typedef enum 
    {
        SAP = 1,    /**< Service Access Point              (port connector)*/
        SPP,        /**< Service Provision Point           (port acceptor)*/
        MSPP,       /**< Multi Service Provision Point     (port provider)*/
    } TPortType;
    
    /** @param protocol NOT USED */
    explicit GCFPortInterface (GCFTask* pTask, 
                      string name, 
                      TPortType type, 
                      int protocol, 
                      bool transportRawData) :
        _pTask(pTask), 
        _name(name), 
        _isConnected(false), 
        _type(type), 
        _protocol(protocol),
        _transportRawData(transportRawData)
    {
    }
    
    virtual ~GCFPortInterface () {};
    
    virtual int close () = 0;
    virtual int open ()  = 0;
    
    /**
    * send/recv functions
    */
    virtual ssize_t send (GCFEvent& event) = 0;
    
    virtual ssize_t recv (void* buf, 
                          size_t count) = 0;
    
    /**
    * Timer functions.
    * Upon expiration of a timer a F_TIMER_SIG will be
    * received on the port.
    */
    virtual long setTimer (long  delay_sec,
                           long  delay_usec    = 0,
                           long  interval_sec  = 0,
                           long  interval_usec = 0,
                           void* arg     = 0) = 0;
    
    virtual long setTimer (double delay_seconds, 
                           double interval_seconds = 0.0,
                           void*  arg        = 0) = 0;
    
    virtual int  cancelTimer (long   timerid,
                              void** arg = 0) = 0;
    
    virtual int  cancelAllTimers() = 0;
    
    virtual int  resetTimerInterval(long timerid,
                                    long sec,
                                    long usec = 0) = 0;
    
    /**
    * Attribute access functions
    */
    inline const string&  getName ()     const {return _name;}
    inline TPortType      getType ()     const {return _type;}
    inline bool           isConnected () const {return _isConnected;}
    inline const GCFTask* getTask ()     const {return _pTask;}
    inline int            getProtocol () const {return _protocol;}
    inline bool           isTransportRawData () const {return _transportRawData;}

  protected:
    GCFTask*  _pTask;
    string    _name;
    bool      _isConnected;
    TPortType _type;
    int       _protocol; /**< NOT USED */
    bool      _transportRawData;
    
    virtual inline void setIsConnected (bool connected) {_isConnected = connected;}
    
    /** @param protocol NOT USED */
    virtual void init(GCFTask& task, 
                      string& name, 
                      TPortType type,  
                      int protocol, 
                      bool transportRawData = false)
    {
      _pTask = &task;
      _name = name;  
      _type = type;
      _protocol = protocol;
      _isConnected = false;
      _transportRawData = transportRawData;
    }
};

#endif
