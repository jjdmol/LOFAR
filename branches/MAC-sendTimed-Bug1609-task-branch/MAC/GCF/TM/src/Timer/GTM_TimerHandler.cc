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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#define LOFARLOGGER_SUBPACKAGE "Timer"

#include <Timer/GTM_TimerHandler.h>
#include <Timer/GTM_Timer.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_RawPort.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

GTMTimerHandler* GTMTimerHandler::_pInstance = 0;

//
// instancce()
//
GTMTimerHandler* GTMTimerHandler::instance()
{
	if (!_pInstance) {
		_pInstance = new GTMTimerHandler();
		ASSERT(!_pInstance->mayDeleted());
	}
	_pInstance->use();
	return (_pInstance);
}

//
// release()
//
void GTMTimerHandler::release()
{
	ASSERT(_pInstance);
	ASSERT(!_pInstance->mayDeleted());
	_pInstance->leave(); 
	if (_pInstance->mayDeleted()) {
		delete _pInstance;
		_pInstance = 0;
	}
}

GTMTimerHandler::GTMTimerHandler() :
  _running(true)
{  
}

GTMTimerHandler::~GTMTimerHandler()
{ 
	GTMTimer* pCurTimer(0);
	TTimers::iterator	iter = _timers.begin();
	TTimers::iterator	end  = _timers.end();
	while (iter != end) {
		pCurTimer = iter->second;
		ASSERT(pCurTimer);
		delete pCurTimer;
		++iter;
	}
	_timers.clear();
	_pInstance = 0;
}

void GTMTimerHandler::stop()
{
	_running = false;
}
 
//
// workProc()
//
void GTMTimerHandler::workProc()
{
	GTMTimer* pCurTimer(0);

	TTimers tempTimers;
	tempTimers.insert(_timers.begin(), _timers.end());

	TTimers::iterator	iter = tempTimers.begin();
	TTimers::iterator	end  = tempTimers.end();
	while (iter != end && _running) {
		pCurTimer = iter->second;
		ASSERT(pCurTimer);
		if (pCurTimer->isElapsed() || pCurTimer->isCanceled()) {
			LOG_TRACE_STAT(formatString("Deleting timer %d(%s),elapse=%c,cancel=%c", iter->first, 
						pCurTimer->getPort().getName().c_str(),
						(pCurTimer->isElapsed() ? 'Y' : 'N'), (pCurTimer->isCanceled() ? 'Y' : 'N')));
			delete pCurTimer;
			_timers.erase(iter->first);
		}
		else {
			pCurTimer->decreaseTime();	// and dispatch F_TIMER when elapsed
		}
		++iter;
	}
}

//
// setTimer(port, delay, interval, arg, userval)
//
unsigned long GTMTimerHandler::setTimer(GCFRawPort& port, 
										uint64	delaySeconds, 
										uint64	intervalSeconds,
										void*	arg,
										uint32	userValue)
{
	unsigned long timerid(1);

	// search the first unused timerid
	TTimers::iterator 	iter;
	TTimers::iterator	end  = _timers.end();
	do {
		timerid++;
		iter = _timers.find(timerid);
	}
	while (iter != end);  

	_timers[timerid] = new GTMTimer(port, timerid, delaySeconds, intervalSeconds, arg, userValue);

	return (timerid);
}

//
// cancelTimer(timerID, arg)
//
int GTMTimerHandler::cancelTimer(unsigned long timerid, void** arg)
{
	// allow timerid 0
	if (!timerid) {
		return (0);
	}

	GTMTimer*	pCurTimer(0);
	TTimers::iterator iter = _timers.find(timerid);
	if (arg) {
		*arg = 0;
	}
	if (iter == _timers.end()) {
		return (0);
	}
	pCurTimer = iter->second;

	ASSERT(pCurTimer);
	if (arg) {
		*arg = pCurTimer->getUserPtr();
	}
	pCurTimer->cancel();		// Note: sets internal flag in Timer.

	return (1);
}

//
// cancelAllTimers(port)
//
int GTMTimerHandler::cancelAllTimers(GCFRawPort& port)
{
	int 		result(0);
	GTMTimer*	pCurTimer(0);

	TTimers::iterator	iter = _timers.begin();
	TTimers::iterator	end  = _timers.end();
	while (iter != end) {
		pCurTimer = iter->second;
		ASSERT(pCurTimer);
		if (&(pCurTimer->getPort()) == &port) {  
			pCurTimer->cancel();
			result++;
		}
		++iter;
	}
	return (result);
}

//
// timeLeft(timerID)
//
double GTMTimerHandler::timeLeft(unsigned long timerID)
{
	GTMTimer* 			pCurTimer(0);
	TTimers::iterator 	iter(_timers.find(timerID));
	if (iter == _timers.end()) {
		return (0.0);
	}

	pCurTimer = iter->second;
	ASSERT(pCurTimer);

	return (pCurTimer->getTimeLeft());
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
