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

#include "GTM_Timer.h"
#include <PortInterface/GCF_RawPort.h>
#include "GTM_TimerHandler.h"

GTMTimer::GTMTimer(GCFRawPort& port, 
                   unsigned long time, 
                   unsigned long intervalTime = 0, 
                   const void* arg = 0):
  _port(port), _time(time), _timeLeft(time), _intervalTime(intervalTime), 
  _arg(arg)
{
  _id = GTMTimerHandler::instance()->registerTimer(*this);
}

int GTMTimer::stop()
{
  return GTMTimerHandler::instance()->deregisterTimer(*this);
}
 
void GTMTimer::decreaseTime(unsigned long microSec)
{
  if (_timeLeft > microSec)
  {
    _timeLeft -= microSec;
  }
  else
  {
    _port.handleTimeout(*this);
    if (_intervalTime > 0)
    {
      _timeLeft = _intervalTime;
    }
    else
    {
      stop();
    }
  }  
}
