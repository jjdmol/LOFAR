//#  GCF_TCPPort.h: connection to a remote process
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

#include <PortInterface/GCF_RawPort.h>
#include <PortInterface/GCF_PeerAddr.h>
#include <Common/lofar_string.h>

// forward declaration
class GCFTask;
class GTMSocket;
class GCFEvent;

class GCFTCPPort : public GCFRawPort
{
 public:

    ////////////////////// Construction methods
    GCFTCPPort(GCFTask& task,
  	    string name,
  	    TPortType type,
        int protocol = 0);
    GCFTCPPort();
  
    virtual ~GCFTCPPort();
  
    ////////////////////// GCFPortInterface methods

  public:

    /**
     * open/close functions
     */
    virtual int open();
    virtual int close();
  
    /**
     * send/recv functions
     */
    virtual ssize_t send(const GCFEvent& event,
  		       void* buf = 0, size_t count = 0);
    virtual ssize_t sendv(const GCFEvent& e,
  			const iovec buffers[], int n);
    virtual ssize_t recv(void* buf,     size_t count);
    virtual ssize_t recvv(iovec buffers[], int n);

  ////////////////////// EOF GCFPortInterface methods
  protected:
    friend class GTMSocket;
    friend class GTMServerSocket;
  
  private:

    /**
     * Don't allow copying of the FPort object.
     */
    GCFTCPPort(const GCFTCPPort&);
    GCFTCPPort& operator=(const GCFTCPPort&);

  public:

    // addr is local address if getType == (M)SPP
    // addr is remote addres if getType == SAP
    void setAddr(const GCFPeerAddr& addr);
    
    string _remotePortname;
  
  private:
    bool                _addrIsSet;
    GTMSocket*          _pSocket;
    GCFPeerAddr         _addr;
};

#endif
