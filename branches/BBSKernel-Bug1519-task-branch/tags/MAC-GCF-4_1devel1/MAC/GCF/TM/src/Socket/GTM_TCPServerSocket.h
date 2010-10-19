//#  GTM_TCPServerSocket.h: the server socket
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

#ifndef GTM_TCPSERVERSOCKET_H
#define GTM_TCPSERVERSOCKET_H

#include <Socket/GTM_TCPSocket.h>

// forward declaration
class GCFTCPPort;
class GCFPeerAddr;
/**
 * This class will be used by a port implementation when its type is (M)SPP. In 
 * case the port type is MSPP it only acts as a provider/acceptor. Otherwise 
 * (SPP) it acts as the message exchange (send/receive) point for a P-t-P 
 * connection too.
 */
class GTMTCPServerSocket : public GTMTCPSocket
{
 public:

    /// Construction methods
    GTMTCPServerSocket (GCFTCPPort& port, 
                     bool isProvider = false);
  
    virtual ~GTMTCPServerSocket ();
  
    /**
     * open/close functions
     */
    virtual int open (unsigned int portNumber);
    virtual int close ();
    int accept (GTMSocket& newSocket);
    
    /**
     * send/recv functions
     */
    virtual ssize_t send (void* buf, size_t count);
    virtual ssize_t recv (void* buf, size_t count);

  protected:
    virtual void workProc ();

  private:
    GTMTCPServerSocket();
  
    /**
     * Don't allow copying of the GTMTCPServerSocket object.
     */
    GTMTCPServerSocket (const GTMTCPServerSocket&);
    GTMTCPServerSocket& operator= (const GTMTCPServerSocket&);
    
    bool _isProvider;
    GTMTCPSocket* _pTCPServerSocket;
};

#endif
