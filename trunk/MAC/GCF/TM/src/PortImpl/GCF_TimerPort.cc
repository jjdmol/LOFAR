//#  GCF_TimerPort.cc: Raw connection to a remote process
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

#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Protocols.h>
#include <GTM_Defines.h>
#include <Timer/GTM_TimerHandler.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

//
// GCFTimerPort()
//
GCFTimerPort::GCFTimerPort(GCFTask&			aTask,
						   const string&	aName) :
    GCFRawPort(&aTask, aName, SAP, 0, false)
{
	_pTimerHandler = GTMTimerHandler::instance(); 
	ASSERT(_pTimerHandler);
}

//
// ~GCFTimerPort()
//
GCFTimerPort::~GCFTimerPort()
{
	cancelAllTimers();
	ASSERT(_pTimerHandler);
	GTMTimerHandler::release();
	_pTimerHandler = 0;
}

//
// dispatch(event)
//
GCFEvent::TResult GCFTimerPort::dispatch(GCFEvent& event)
{
	ASSERT(_pTask);
	return (_pTask->dispatch(event, *this));
}

//
// settimer(sec, usec, itvsec, itvusec, arg)
//
long GCFTimerPort::setTimer(long delay_sec, long delay_usec,
						  long interval_sec, long interval_usec,
						  void* arg)
{
	ASSERT(_pTimerHandler);
	uint64 	delay(delay_sec);
	uint64 	interval(interval_sec);
	delay    *= 1000000;
	interval *= 1000000;
	delay    += (uint64) delay_usec;
	interval += (uint64) interval_usec;

	return (_pTimerHandler->setTimer(*this, delay, interval, arg));  
}

//
// setTimer(sec, itvsec, arg)
//
long GCFTimerPort::setTimer(double delay_seconds, 
						  double interval_seconds,
						  void* arg)
{
  ASSERT(_pTimerHandler);

  return (_pTimerHandler->setTimer(*this, 
									 (uint64) (delay_seconds * 1000000.0), 
									 (uint64) (interval_seconds * 1000000.0),
									 arg));
}

//
// cancelTimer(timerid, arg)
//
int GCFTimerPort::cancelTimer(long timerid, void **arg)
{
  ASSERT(_pTimerHandler);
  return _pTimerHandler->cancelTimer(timerid, arg);
}

//
// cancelAllTimers()
//
int GCFTimerPort::cancelAllTimers()
{
  ASSERT(_pTimerHandler);
  return _pTimerHandler->cancelAllTimers(*this);
}


  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
