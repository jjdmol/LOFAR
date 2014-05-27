//#  GCFFsm.cc: implementation of Finite State Machine.
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
#include <GCF/TM/GCF_Fsm.h>
#include <GCF/TM/GCF_Scheduler.h>

namespace LOFAR {
 using MACIO::GCFEvent;
 namespace GCF {
  namespace TM {

//
// GCFFsm(state)
//
GCFFsm::GCFFsm (State initial) : 
	itsState	(initial),
	itsScheduler(GCFScheduler::instance())
{
}

//
// initFsm()
//
void GCFFsm::initFsm()
{
	GCFEvent	entryEvent(F_ENTRY);
	itsScheduler->queueEvent(this, entryEvent, 0);

	GCFEvent	initEvent(F_INIT);
	itsScheduler->queueEvent(this, initEvent, 0);
}

//
// tran(target, from, to)
//
void GCFFsm::tran(State target, const char* from, const char* to)
{
	itsScheduler->queueTransition(this, target, from, to);
}

//
// quitFsm()
//
void GCFFsm::quitFsm()
{
	GCFEvent	quitEvent(F_QUIT);
	itsScheduler->queueEvent(this, quitEvent, 0);
}

//
// queueTaskEvent(event,port)
//
// Add the given event/port pair to the eventqueue of the task. This queue is emptied as soon as the
// state of the task is switched.
//
void GCFFsm::queueTaskEvent(GCFEvent&	event, GCFPortInterface&	port)
{
	waitingTaskEvent_t*		newEntry = new waitingTaskEvent_t;
	newEntry->event = event.clone();
	newEntry->port  = &port;
	itsEventQueue.push_back(newEntry);
}

//
// unqueueTaskEvent(&event,&port)
//
// Removes and returns the most recent task from the taskQueue
//
bool GCFFsm::unqueueTaskEvent(GCFEvent**	eventPtr, GCFPortInterface**portPtr)
{
	if (itsEventQueue.empty()) {
		return (false);
	}

	waitingTaskEvent_t*		newestEvent = itsEventQueue.back();
	*eventPtr = newestEvent->event;
	*portPtr  = newestEvent->port;
	itsEventQueue.pop_back();
	delete newestEvent;		// NOTE: DON't delete newestEvent->event, let the user do that.
	return(true);
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
