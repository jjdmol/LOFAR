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

#include <Timer/GTM_TimerHandler.h>
#include <Timer/GTM_Timer.h>
#include <GCF/TM/GCF_Task.h>

GTMTimerHandler* GTMTimerHandler::_pInstance = 0;

GTMTimerHandler* GTMTimerHandler::instance()
{
  if (0 == _pInstance)
  {
    _pInstance = new GTMTimerHandler();
  }

  return _pInstance;
}

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

  map<unsigned long, GTMTimer*> tempTimers;
  
  tempTimers.insert(_timers.begin(), _timers.end());
  if (!tempTimers.empty())
  {
    timeval curTime;
    struct timezone timeZone;
    gettimeofday(&curTime, &timeZone);
    microSecDiff = ((unsigned long) (curTime.tv_usec) + 
                    (unsigned long) (curTime.tv_sec) * 1000000) - 
                   ((unsigned long) (_lastTime.tv_usec) +
                    (unsigned long) (_lastTime.tv_sec) * 1000000);
  }
  for (TTimerIter iter = tempTimers.begin(); iter != tempTimers.end() && _running; ++iter)
  {
    pCurTimer = iter->second;
    if (pCurTimer)
    {
      if (pCurTimer->isElapsed() || pCurTimer->isCanceled())
      {
        delete pCurTimer;
        _timers.erase(iter->first);
      }
      else
        pCurTimer->decreaseTime(microSecDiff);             
    }  
  }
  if (!_timers.empty())
  {
    saveDateTime();
  }
}

void GTMTimerHandler::saveDateTime()
{
  struct timezone timeZone;
  time_t now = time(NULL);
  localtime_r(&now, &_lastDateTime);
  gettimeofday(&_lastTime, &timeZone);
}


unsigned long GTMTimerHandler::setTimer(GCFRawPort& port, 
					unsigned long delay_seconds, 
					unsigned long interval_seconds,
					void*  arg)
{
  unsigned long timerid(0);

  if (_timers.empty()) saveDateTime(); // start timer

  // search the first unused timerid
  TTimerIter iter;
  do 
  {
    timerid++;
    iter = _timers.find(timerid);
  }
  while (iter != _timers.end());  

  GTMTimer* pNewTimer = new GTMTimer(port, 
				     timerid,
				     delay_seconds, 
				     interval_seconds,
				     arg);
  _timers[timerid] = pNewTimer;

  return timerid;
}

int GTMTimerHandler::cancelTimer(unsigned long timerid, void** arg)
{
  int result(0);
  GTMTimer* pCurTimer(0);
  TTimerIter iter = _timers.find(timerid);
  if (iter == _timers.end())
    return result;
  pCurTimer = iter->second; //second is of type GTMTimer*
  if (pCurTimer)
  {
    result = 1;
    if (arg)
      *arg = pCurTimer->getTimerArg();
    pCurTimer->cancel();
  }
  
  return result;
}

int GTMTimerHandler::cancelAllTimers(GCFRawPort& port)
{
  int result(0);
  GTMTimer* pCurTimer(0);
  
  for (TTimerIter iter = _timers.begin(); 
       iter != _timers.end(); ++iter)
  {
    pCurTimer = iter->second;
    if (pCurTimer)
    {
      if (&(pCurTimer->getPort()) == &port)
      {  
        pCurTimer->cancel();
        if (!result) result = 1;
      }
    }
  }
  return result;
}
