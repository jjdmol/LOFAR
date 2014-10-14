//
//  tTimer.cc: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2006
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include "tTimer.h"

static	int	gTest = 0;
static	int	gTimerID;

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tTimer::tTimer(string name) : 
	GCFTask         ((State)&tTimer::test1, name),
	itsTimerPort    (0)
{ 
}

tTimer::~tTimer()
{
	delete itsTimerPort;
}

//
// TEST 1
//
// The server opens a TCP port with name SERVER:test1 using the settings in
// the configuration file.
// The client tries to resolve the label CLIENT:1stTest into the right
// service name of the server using the config files and serviceBroker.
//
GCFEvent::TResult tTimer::test1(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tTimer::test1: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort = new GCFTimerPort(*this, "timerPort");
		ASSERTSTR(itsTimerPort, "Failed to open timerport");
		break;

	case F_INIT:
		gTest=1;
		gTimerID = itsTimerPort->setTimer(1.0);
		LOG_DEBUG_STR("setTimer(1.0) = " << gTimerID);
		break;

	case F_TIMER: {
		switch (gTest) {
			case 1: {	// wait for setTimer(1.0)
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);

				gTimerID = itsTimerPort->setTimer(1.0, 2.0);
				LOG_DEBUG_STR("setTimer(1.0, 2.0) = " << gTimerID);
				gTest++;
			}
			break;

			case 2: { // wait for first expire of setTimer(1.0, 2.0)
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);
				gTest++;
			}
			break;

			case 3: {	// wait for second expire of setTimer(1.0, 2.0)
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);
				itsTimerPort->cancelTimer(gTimerID);

				gTimerID = itsTimerPort->setTimer(1.0, 1.0, (char*)"pietje puk");
				LOG_DEBUG_STR("setTimer(1.0, 0.0, 'pietje puk') = " << gTimerID);
				gTest++;
			}
			break;
			case 4: {
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", arg = " << timerEvent.arg);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", *arg = " << (char*)timerEvent.arg);
				gTest++;
			}
			break;

			default: {
				GCFTimerEvent& timerEvent = static_cast<GCFTimerEvent&>(event);
				LOG_DEBUG_STR("ID = " << timerEvent.id << ", *arg = " << (char*)timerEvent.arg);
				if (gTest++ > 20) {
					itsTimerPort->cancelTimer(gTimerID);
					GCFScheduler::instance()->stop();
				}
			}
			break;
		}
		break;
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF::TM;

//
// MAIN()
//
int main(int argc, char* argv[])
{
	GCFScheduler::instance()->init(argc, argv);

	tTimer	timerTask("TimerTask");

	timerTask.start(); // make initial transition

	GCFScheduler::instance()->run();

	return (0);
}
