//#  GTM_Socket.h: base class for all sockets
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

#ifndef GTM_SOCKET_H
#define GTM_SOCKET_H

#include "unistd.h"
#include <GCF_Event.h>

// forward declaration
class GCFTCPPort;
class GCFPeerAddr;
class GTMSocketHandler;
class GTMServerSocket;


class GTMSocket
{
  public:
    GTMSocket(GCFTCPPort& port);
    virtual ~GTMSocket();
  
    /**
     * open/close functions
     */
    virtual int open(GCFPeerAddr& addr);
    virtual int close();
    virtual int connect(GCFPeerAddr& addr);
  
    /**
     * send/recv functions
     */
    virtual ssize_t send(void* buf, size_t count);
    virtual ssize_t recv(void* buf, size_t count);

    virtual inline int getFD() const {return _socketFD;}
    
  protected:

    virtual int setFD(int fd);
    virtual void workProc();
    friend class GTMSocketHandler;
    friend class GTMServerSocket;

    GCFTCPPort&   _port;
    int           _socketFD;
  
  private:
    GTMSocket();
    /**
     * Don't allow copying of the GTMSocket object.
     */
    GTMSocket(const GTMSocket&);
    GTMSocket& operator=(const GTMSocket&);
    GCFEvent::TResult eventReceived(const GCFEvent& e);
};

#endif
