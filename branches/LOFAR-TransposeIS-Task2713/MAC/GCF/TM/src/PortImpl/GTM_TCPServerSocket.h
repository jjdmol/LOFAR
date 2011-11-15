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

#include "GTM_TCPSocket.h"

namespace LOFAR {
 namespace GCF {
  namespace TM {

// forward declaration

/**
 * This class will be used by a TCP port implementation when its type is (M)SPP. In 
 * case the port type is MSPP it only acts as a provider/acceptor. Otherwise 
 * (SPP) it acts as the message exchange (send/receive) point for a P-t-P 
 * connection too.
 */
class GTMTCPServerSocket : public GTMTCPSocket
{
public: 
	// constructors, destructors and default operators
    GTMTCPServerSocket (GCFTCPPort& port, 
                        bool isProvider = false);
    virtual ~GTMTCPServerSocket ();
  
	// GTMTCPServerSocket specific member methods
    // open/close methods
    virtual bool open (unsigned int portNumber);
    virtual bool close ();
    bool accept (GTMFile& newSocket);
    
    // send/recv methods
    virtual ssize_t send (void* buf, size_t count);
    virtual ssize_t recv (void* buf, size_t count, bool raw = false);

private:
    GTMTCPServerSocket();
    /// Don't allow copying of the GTMTCPServerSocket object.
    GTMTCPServerSocket (const GTMTCPServerSocket&);
    GTMTCPServerSocket& operator= (const GTMTCPServerSocket&);

protected:
    virtual void doWork();
    
    bool 			_isProvider;
    GTMTCPSocket*	_pDataSocket;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
