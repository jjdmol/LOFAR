//#  GCF_TCPPort.h: TCP connection to a remote process
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

#ifndef GCF_TCPPORT_H
#define GCF_TCPPORT_H

#include <Common/SystemUtil.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_TimerPort.h>

namespace LOFAR {
 class MACIO::GCFEvent;
 namespace GCF {
  namespace SB {
	class ServiceBrokerTask;
  }
  
  namespace TM {

// forward declaration
class GCFTask;
class GTMTCPSocket;

/**
 * This is the class, which implements the special port with the TCP message 
 * transport protocol. It uses socket pattern to do this. Is can act as MSPP 
 * (port provider), SPP (server) and SAP (client).
 */
class GCFTCPPort : public GCFRawPort
{
public:// consturctors && destructors
    /// params see constructor of GCFPortInterface    
    GCFTCPPort (GCFTask& 		task,
						 const string&	name,
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
    GCFTCPPort ();
  
private:  
    /// copying is not allowed.
    GCFTCPPort (const GCFTCPPort&);
    GCFTCPPort& operator= (const GCFTCPPort&);

public:
    /// desctructor
    virtual ~GCFTCPPort ();
  
public: // GCFPortInterface overloaded/defined methods
    void init (GCFTask& 	 task, 
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
     * send/recv functions
     */
    virtual ssize_t send (GCFEvent& event);
    virtual ssize_t recv (void* buf,
                          size_t count);
public: // GCFTCPPort specific methods    
    virtual bool accept (GCFTCPPort& port); 
    // addr is local address if getType == (M)SPP
    // addr is remote addres if getType == SAP
    void setAddr (const TPeerAddr& addr);
    void setHostName(const string& hostname);
    void setPortNumber(unsigned int portNumber);
    string getHostName();
    unsigned int getPortNumber();

private: // helper methods
    friend class SB::ServiceBrokerTask;
    void serviceRegistered(unsigned int result, unsigned int portNumber);
    void serviceUnregistered();
    void serviceInfo(unsigned int result, unsigned int portNumber, const string& host);
    void serviceGone();
    
protected: // data members
    GTMTCPSocket*   _pSocket;

private:
    bool					_addrIsSet;
    TPeerAddr				_addr;
    string					_host;
    unsigned int			_portNumber;
	bool					itsFixedPortNr;
    SB::ServiceBrokerTask*	_broker;
};

inline void GCFTCPPort::setHostName(const string& hostname)
{
	// assure that hostname is never filled with localname.
	if (hostname.empty() || hostname == "localhost") {
		_host = myHostname(false);
	}
	else {
		_host = hostname;
	}
}

inline void GCFTCPPort::setPortNumber(unsigned int portNumber)
{
	_portNumber = portNumber;
	itsFixedPortNr = true;
}

inline string GCFTCPPort::getHostName()
{
  return _host;
}

inline unsigned int GCFTCPPort::getPortNumber()
{
  return _portNumber;
}
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
