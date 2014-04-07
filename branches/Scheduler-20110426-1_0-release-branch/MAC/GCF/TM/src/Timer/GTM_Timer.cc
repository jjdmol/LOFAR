
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#define LOFARLOGGER_SUBPACKAGE "Timer"

#include <Timer/GTM_Timer.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GCF/TM/GCF_Scheduler.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

GTMTimer::GTMTimer(GCFRawPort& port, 
            		   unsigned long id,
                   uint64 timeVal, 
                   uint64 intervalTime, 
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
	int64 uSec = getElapsedTime();

	// REO: uSec < 0 ??? 
	if ((uint64) uSec < _timeLeft || uSec < 0) {
		_timeLeft -= uSec;
		return;
	}

	// timer expired
	struct timeval now;
	(void)gettimeofday(&now, NULL);

	GCFTimerEvent te;
	te.sec  = now.tv_sec;
	te.usec = now.tv_usec;
	te.id   = _id;
	te.arg  = _arg;
	GCFScheduler::instance()->queueEvent(0, te, &_port);

	if (_intervalTime == 0) {
		_elapsed = true;
		return;
	}

	uint64 timeoverflow = uSec - _timeLeft;
	if (_intervalTime < timeoverflow) {
		LOG_ERROR(formatString(
		"Timerinterval %fsec of timer %d is to small for performance reasons (tdelta: %lld, tleft: %llu).",
		((double) _intervalTime) / 1000000.0, _id, uSec, _timeLeft));
		do {
		  timeoverflow -= _intervalTime;
		} while (_intervalTime < timeoverflow);        
	}

	_timeLeft = _intervalTime - timeoverflow;
}

void GTMTimer::saveTime()
{
	struct timezone timeZone;
	gettimeofday(&_savedTime, &timeZone);
}

int64 GTMTimer::getElapsedTime()
{
	timeval oldTime(_savedTime);
	int64 	uSecDiff(0);

	saveTime();

	uSecDiff = ((uint64) (_savedTime.tv_usec) + 
				(uint64) (_savedTime.tv_sec) * 1000000) - 
			   ((uint64) (oldTime.tv_usec) +
				(uint64) (oldTime.tv_sec) * 1000000);

	return (uSecDiff);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
