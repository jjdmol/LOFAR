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

#include <unistd.h>
#include <GCF/GCF_Event.h>

// forward declaration
class GCFPeerAddr;
class GTMSocketHandler;
class GCFRawPort;

/**
 * This class consists of the basic implementation of a socket. Beside that it 
 * is the base class for the GTMServerSocket class.
 */

class GTMSocket
{
  public:
    GTMSocket (GCFRawPort& port);
    virtual ~GTMSocket ();
  
    /**
     * open/close functions
     */
    virtual int close ();
  
    /**
     * send/recv functions
     */
    virtual ssize_t send (void* buf, size_t count) = 0;
    virtual ssize_t recv (void* buf, size_t count) = 0;

    virtual inline int getFD () const {return _socketFD;}
    virtual int setFD (int fd);
    virtual void workProc ();
    
  protected:
    int           _socketFD;
    GCFRawPort&   _port;    
  
  private:
    /**
     * Don't allow copying of the GTMSocket object.
     */
    GTMSocket (const GTMSocket&);
    GTMSocket& operator= (const GTMSocket&);
};

#endif
