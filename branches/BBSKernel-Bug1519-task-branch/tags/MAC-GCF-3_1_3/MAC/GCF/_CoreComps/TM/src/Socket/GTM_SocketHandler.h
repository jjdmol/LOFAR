//#  GTM_SocketHandler.h: handles all socket communication
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

#ifndef GTM_SOCKETHANDLER_H
#define GTM_SOCKETHANDLER_H

#include <GCF/GCF_Handler.h>
#include <Common/lofar_map.h>
#include <sys/time.h>

// forward declaration
class GTMSocket;

/**
 * This singleton class implements the main loop part of message exchange 
 * handling, which uses the socket pattern. It calls one select for all file 
 * descriptors of the registered sockets.
 */
class GTMSocketHandler : public GCFHandler
{
  public:
    static GTMSocketHandler* instance ();
    virtual ~GTMSocketHandler () {};
  
    void workProc ();
    void stop ();
    void registerSocket (GTMSocket& timer);
    void deregisterSocket (GTMSocket& timer); 

  private:
    GTMSocketHandler ();
    /**
     * Don't allow copying of the GTMTimerHandler object.
     */
    GTMSocketHandler (const GTMSocketHandler&);
    GTMSocketHandler& operator= (const GTMSocketHandler&);
    static GTMSocketHandler* _pInstance;
    
    map<int, GTMSocket*> _sockets;
    typedef map<int, GTMSocket*>::iterator TSocketIter;

    
    fd_set _readFDs;
    fd_set _writeFDs;
    fd_set _errotFDs;
    bool _running;
    
};

#endif
