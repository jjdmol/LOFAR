//#  GTM_ServerSocket.h: the server socket
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

#ifndef GTM_SERVERSOCKET_H
#define GTM_SERVERSOCKET_H

#include "GTM_Socket.h"
// forward declaration
class GCFRawPort;
class GCFPeerAddr;

class GTMServerSocket : public GTMSocket
{
 public:

    ////////////////////// Construction methods
    GTMServerSocket(GCFRawPort& port, bool isProvider = false);
  
    virtual ~GTMServerSocket();
  
    /**
     * open/close functions
     */
    virtual int open(GCFPeerAddr& addr);
    virtual int close();
    void accept(GTMSocket& newSocket);
    

  protected:
    void workProc();

  private:
    GTMServerSocket();
  
    /**
     * Don't allow copying of the FPort object.
     */
    GTMServerSocket(const GTMServerSocket&);
    GTMServerSocket& operator=(const GTMServerSocket&);
    
    bool _isProvider;
    int _serverSocketFD;
};

#endif
