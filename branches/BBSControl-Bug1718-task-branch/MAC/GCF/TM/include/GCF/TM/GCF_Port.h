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

#include <GCF/TM/GCF_PortInterface.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

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
  public: // constructor && destructor

    /// params see constructor of GCFPortInterface
    explicit GCFPort (GCFTask& 		containertask, 
					  const string& name, 
					  TPortType 	type, 
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
    GCFPort ();
    
    /// destructor
    virtual ~GCFPort ();
  
  private:

    /// Don't allow copying this object.
    GCFPort (const GCFPort&);
    GCFPort& operator= (const GCFPort&);

  public: // GCFPortInterface overloaded/defined methods
    
    /// params see constructor of GCFPortInterface
    void init (GCFTask& 	 containertask, 
			   const string& name, 
			   TPortType 	 type, 
			   int 			 protocol, 
			   bool 		 transportRawData = false);
    
    /**
    * open/close methods
    */
    virtual bool open ();
    virtual bool close ();
    
    /**
    * send/recv methods
    */
    virtual ssize_t send (LOFAR::MACIO::GCFEvent& event);
    virtual ssize_t recv (void* buf, 
                          size_t count);
    
    /**
     * these methods forwards the timer functionality requests to the special port classes
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

	virtual double timeLeft(long	timerID);
    
  public: // GCFPort specific interface methods
    /// sets the remote address of a port 
    virtual bool setRemoteAddr(const string& remotetask, const string& remoteport);

 
  private: // data members
    string 				_remotetask;
    string 				_remoteport;
    GCFPortInterface* 	_pSlave;

    friend class GCFRawPort; // to access the setState method of the base class
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
