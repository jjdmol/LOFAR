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

#include <GCF/TM/GCF_RawPort.h>
#include <Common/lofar_string.h>

// forward declaration
class GCFTask;
class GCFEvent;
class GTMTCPSocket;
class GTMServiceBroker;

/**
 * This is the class, which implements the special port with the TCP message 
 * transport protocol. It uses socket pattern to do this. Is can act as MSPP 
 * (port provider), SPP (server) and SAP (client).
 */
class GCFTCPPort : public GCFRawPort
{
 public:

    /// Construction methods
    /** @param protocol NOT USED */    
    explicit GCFTCPPort (GCFTask& task,
          	    string name,
          	    TPortType type,
                int protocol, 
                bool transportRawData = false);
    explicit GCFTCPPort ();
  
    virtual ~GCFTCPPort ();
  
  public:

    /**
     * open/close functions
     */
    virtual bool open ();
    virtual bool close ();
  
    virtual bool accept (GCFTCPPort& port);
    /**
     * send/recv functions
     */
    virtual ssize_t send (GCFEvent& event);
    virtual ssize_t recv (void* buf,
                          size_t count);
  public:

    // addr is local address if getType == (M)SPP
    // addr is remote addres if getType == SAP
    void setAddr (const TPeerAddr& addr);

  private:
    /**
     * Don't allow copying this object.
     */
    GCFTCPPort (const GCFTCPPort&);
    GCFTCPPort& operator= (const GCFTCPPort&);
    
    friend class GTMServiceBroker;
    void serviceRegistered(unsigned int result, unsigned int portNumber);
    void serviceUnregistered();
    void serviceInfo(unsigned int result, unsigned int portNumber, const string& host);
    void serviceGone();
    
  protected:
    GTMTCPSocket*   _pSocket;

  private:
    bool            _addrIsSet;
    TPeerAddr       _addr;
    string          _host;
    unsigned int    _portNumber;
    GTMServiceBroker* _broker;
};

#endif
