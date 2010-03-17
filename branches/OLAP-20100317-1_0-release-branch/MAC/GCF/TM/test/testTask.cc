//#  testTask.cc: one_line_description
//#
//#  Copyright (C) 2002-2004
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_Protocols.h>
#include "testTask.h"

namespace LOFAR {
  namespace GCF {
    namespace TM {

//
// testTask(name, timerInterval)
//
testTask::testTask(const string&	name, double	timerInterval) :
	GCFTask			((State)&testTask::mainTask, name),
	itsTimerPort	(0),
	itsTimerInterval(timerInterval),
	itsStopTimer	(0)
{
	LOG_DEBUG_STR("Creation of " << name);
}


//
// ~testTask()
//
testTask::~testTask()
{
	LOG_DEBUG_STR("Destruction of " << getName());

	if (itsTimerPort) {
		delete itsTimerPort;
	}
}

//
// mainTask (event, port)
//
GCFEvent::TResult	testTask::mainTask(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR(getName() << ":" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		break;

	case F_INIT:
		itsTimerPort = new GCFTimerPort(*this, "timerPort");
		itsTimerPort->setTimer(itsTimerInterval, itsTimerInterval);
		if (getName() == "autoQuit") {
			itsStopTimer = itsTimerPort->setTimer(11.3);
		}
		break;

	case F_TIMER:
		LOG_INFO_STR(getName() << ": timer expired");
		if (itsStopTimer) {
			GCFTimerEvent&		timerEvent = static_cast<GCFTimerEvent&>(event);
			if (timerEvent.id == itsStopTimer) {
				LOG_INFO("STOPTIMER EXPIRED, CALLING STOP()");
				GCFScheduler::instance()->stop();
			}
		}
		break;

	case F_QUIT:
		// give testTask different behaviour depending on timerIntervan and name.
		// the Fast timer will not be stopped.
		LOG_INFO_STR(getName() << ": QUIT received");
		if (itsTimerInterval < 1.0) {
			LOG_INFO_STR(getName() << ": Not stopping my timer");
		}
		// the 'medium' timer will be deleted without being cancelled.
		else if (getName() == "medium") {
			LOG_INFO_STR(getName() << ": Deleting my timer");
			itsTimerPort->close();
			delete itsTimerPort;
			itsTimerPort = 0;
		}
		else {
			LOG_INFO_STR(getName() << ": Canceling my timer");
			itsTimerPort->cancelAllTimers();
		}

		break;

	case F_EXIT:
		break;
	default:
		break;
	}
	return (GCFEvent::HANDLED);
}

	} // namespace TM
  } // namespace GCF
} // namespace LOFAR
