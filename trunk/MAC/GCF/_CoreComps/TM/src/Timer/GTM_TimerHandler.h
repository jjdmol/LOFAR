//#  GTM_TimerHandler.h: describes the timeout handler for all running timers
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

#ifndef GTM_TIMERHANDLER_H
#define GTM_TIMERHANDLER_H

#include <GCF_Handler.h>
#include <Common/lofar_map.h>
#include <sys/time.h>
#include <time.h>

class GTMTimer;

class GTMTimerHandler : GCFHandler
{
  public:
    void workProc();
    void stop();
    static GTMTimerHandler* instance();

  private:
    GTMTimerHandler();
    virtual ~GTMTimerHandler() {};
    /**
     * Don't allow copying of the GTMTimerHandler object.
     */
    GTMTimerHandler(const GTMTimerHandler&);
    GTMTimerHandler& operator=(const GTMTimerHandler&);
    static GTMTimerHandler* _pInstance;
    
    map<unsigned long, GTMTimer*> _timers;
    typedef map<unsigned long, GTMTimer*>::iterator TTimerIter;

    friend class GTMTimer;
    unsigned long registerTimer(GTMTimer& timer);
    int deregisterTimer(GTMTimer& timer);
    void saveDateTime();
    
    timeval _lastTime;
    struct tm _lastDateTime;
    bool  _running;
};
#endif
