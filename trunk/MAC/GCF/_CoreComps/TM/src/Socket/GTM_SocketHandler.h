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

#include <GCF_Hanlder.h>

// forward declaration
class GTMSocket;

class GTMSocketHandler : public GCFHandler
{
  public:
  
    void workProc();
    void stop();
    static GTMSocketHandler* instance();

  private:
    GTMSocketHandler();
    virtual ~GTMSocketHandler() {};
    /**
     * Don't allow copying of the GTMTimerHandler object.
     */
    GTMSocketHandler(const GTMSocketHandler&);
    GTMSocketHandler& operator=(const GTMSocketHandler&);
    static GTMSocketHandler* _pInstance;
    
    map<int, GTMSocket*> _timers;
    typedef map<int, GTMSocket*>::iterator TSocketIter;

    friend class GTMSocket;
    void registerSocket(GTMSocket& timer);
    void deregisterSocket(GTMSocket& timer); 
};

#endif
