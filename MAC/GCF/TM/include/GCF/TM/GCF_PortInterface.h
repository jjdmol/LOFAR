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

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <MACIO/GCF_Event.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {
	using LOFAR::MACIO::GCFEvent;

// forward declacations
class GCFTask;

/**
 * This is the abstract base class for all port implementations like TCP or shared 
 * memory. It provides the possibility to:
 * - send and receive events to/from peers in a generic way.
 * - start and stop timers on the port
 */
class GCFPortInterface
{
public:
    /** port types */
    typedef enum TPortType {
        SAP = 1,    /**< Service Access Point              (port connector)*/
        SPP,        /**< Service Provision Point           (port acceptor)*/
        MSPP,       /**< Multi Service Provision Point     (port provider)*/
    };
    
    /** port states */
    typedef enum TSTATE { 
      S_DISCONNECTED, 
      S_CONNECTING, 
      S_CONNECTED,
      S_DISCONNECTING, 
      S_CLOSING
    };
   
    /// destructor
    virtual ~GCFPortInterface ();
    
    virtual bool close () = 0;
    virtual bool open ()  = 0;
    
    /**
    * send/recv functions
    */
    virtual ssize_t send (GCFEvent& event) = 0;
    
    virtual ssize_t recv (void* buf, 
                          size_t count) = 0;
    
    /**
    * Timer functions.
    * Upon expiration of a timer a F_TIMER will be
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
        
	virtual double	timeLeft(long	timerID) = 0;

    virtual GCFEvent::TResult   dispatch  (GCFEvent& event);

    /**
    * Attribute access functions
    */
    const string&	getName ()     	const 		{ return _name; }
    void			setName(const string& name) { _name = name; }
    TPortType		getType ()     	const 		{ return _type; }
    bool			isConnected () 	const 		{ return _state==S_CONNECTED; }
    TSTATE			getState ()    	const 		{ return _state; }
    const GCFTask*	getTask ()     	const 		{ return _pTask; }
    int32			getProtocol () 	const 		{ return _protocol; }
    bool			isTransportRawData () const { return _transportRawData; }
	uint32			getInstanceNr()	const 		{ return _instanceNr; }
	void			setInstanceNr(uint32	nr) { _instanceNr = nr; }
	string 			makeServiceName() const;
	bool			usesModernServiceName() const
					{ return (!_deviceNameMask.empty()); }

protected: 
	// constructors
    /**
     * @param pTask task on which the port is adapted
     * @param name name of the port
     * @param type port type
     * @param protocol NOT USED 
     * @param transportRawData indicates wether the user of this port is only 
     * interested in an indication that there is data (F_DATAIN event) and thus 
     * not in the received data (unpacked in a GCFEvent) itself. In case of F_DATAIN
     * the user is responsible to flush the data from the incomming event buffer
     */
    GCFPortInterface (GCFTask* 	 pTask, 
					   const string& name, 
					   TPortType 	 type, 
					   int 		  	 protocol, 
					   bool 		 transportRawData);

	// helper methods    
    virtual void setState (TSTATE newState) {_state = newState;}
    
    /** params see constructor */
    virtual void init(GCFTask& 		task, 
					  const string& name, 
					  TPortType 	type,  
					  int 			protocol, 
					  bool 			transportRawData = false);

	// data members
    GCFTask*	_pTask;
    string		_name;
	string		_deviceNameMask;
    TSTATE		_state;
    TPortType	_type;
    int32		_protocol; /**< NOT USED */
    bool		_transportRawData;
	uint32		_instanceNr;

private:
    /// default constructor is not allowed
    GCFPortInterface();
    /// copying is not allowed
    GCFPortInterface (GCFPortInterface&);
    GCFPortInterface& operator=(GCFPortInterface&);

};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
