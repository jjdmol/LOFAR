//#  GTM_TimerHandler.cc: describes the timeout handler for all running timers
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

#include "GTM_TimerHandler.h"
#include "GTM_Timer.h"
#include <GCF_Task.h>

GTMTimerHandler::GTMTimerHandler() :
  GCFHandler(), _running(true)
{  
  GCFTask::registerHandler(*this);
}

void GTMTimerHandler::stop()
{
  _running = false;
}
 
void GTMTimerHandler::workProc()
{
  GTMTimer* pCurTimer(0);
  unsigned long microSecDiff(0);
  
  if (!_timers.empty())
  {
    timeval curTime;
    struct timezone timeZone;
    gettimeofday(&curTime, &timeZone);
    microSecDiff = ((unsigned long) (curTime.tv_usec) + 
                    (unsigned long) (curTime.tv_sec) * 1000000) - 
                   ((unsigned long) (_lastTime.tv_usec) +
                    (unsigned long) (_lastTime.tv_sec) * 1000000);
  }
  for (TTimerIter iter = _timers.begin(); iter != _timers.end(); ++iter)
  {
    pCurTimer = iter->second;
    if (pCurTimer)
    {
      pCurTimer->decreaseTime(microSecDiff);
    }  
  }
  if (!_timers.empty())
  {
    saveDateTime();
  }
}

unsigned long GTMTimerHandler::registerTimer(GTMTimer& timer)
{
  unsigned long timerid;

  if (_timers.empty()) saveDateTime(); // start timer
  
  for (timerid = 0; 0 < _timers.count(timerid); timerid++);
  _timers.insert(_timers.begin(), make_pair(timerid, &timer));
  return timerid;
}

int GTMTimerHandler::deregisterTimer(GTMTimer& timer)
{
  return (_timers.erase(timer.getID()) == 1 ? 1 : 0);
}

void GTMTimerHandler::saveDateTime()
{
  struct timezone timeZone;
  time_t now = time(NULL);
  localtime_r(&now, &_lastDateTime);
  gettimeofday(&_lastTime, &timeZone);
}
