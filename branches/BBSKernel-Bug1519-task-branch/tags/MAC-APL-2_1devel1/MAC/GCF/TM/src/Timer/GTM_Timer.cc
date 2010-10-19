//#  GTM_Timer.cc: one line description
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

#include <Timer/GTM_Timer.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GTM_Defines.h>

GTMTimer::GTMTimer(GCFRawPort& port, 
		   unsigned long id,
                   unsigned long timeVal, 
                   unsigned long intervalTime, 
                   void* arg) :
  _port(port), _id(id), _time(timeVal),
  _timeLeft(timeVal), 
  _intervalTime(intervalTime), 
  _arg(arg), _elapsed(false), _canceled(false)
{
  saveTime();
}
 
void GTMTimer::decreaseTime()
{
  unsigned long uSec = getElapsedTime();
  
  if (_timeLeft > uSec)
  {
    _timeLeft -= uSec;
  }
  else
  {
    struct timeval now;
    (void)gettimeofday(&now, NULL);

    GCFTimerEvent te;
    te.sec = now.tv_sec;
    te.usec = now.tv_usec;
    te.id = _id;
    te.arg = _arg;
    
    _port.dispatch(te);
    if (_intervalTime > 0)
    {
      unsigned long timeoverflow = uSec - _timeLeft;
      
      if (_intervalTime < timeoverflow)
      {
        LOG_ERROR(LOFAR::formatString(
            "Timerinterval %fsec of timer %d is to small for performance reasons.",
            ((double) _intervalTime) / 1000000.0,
            _id));
        do 
        {
          timeoverflow -= _intervalTime;
        } while (_intervalTime < timeoverflow);        
      }
      _timeLeft = _intervalTime - timeoverflow;
    }
    else
    {
      _elapsed = true;
    }
  } 
}

void GTMTimer::saveTime()
{
  struct timezone timeZone;
  gettimeofday(&_savedTime, &timeZone);
}

unsigned long GTMTimer::getElapsedTime()
{
  timeval oldTime(_savedTime);
  unsigned long uSecDiff(0);
  
  saveTime();

  uSecDiff = ((unsigned long) (_savedTime.tv_usec) + 
                  (unsigned long) (_savedTime.tv_sec) * 1000000) - 
                 ((unsigned long) (oldTime.tv_usec) +
                  (unsigned long) (oldTime.tv_sec) * 1000000);

  return uSecDiff;                 
}
