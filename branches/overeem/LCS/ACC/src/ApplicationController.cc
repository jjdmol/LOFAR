//#  ApplicationController.cc: Implements the ACC application controller.
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
//#  $Id$
//#
//#  This is a RAW prototype of the application controller that may be used in
//#	 the ACC project. It is not complete nor is it compilable yet.
//#  It just outlines the rough idea how the AC could be build.

#include <ACC/ProcessController.h>
#include <ACC/ProcessState.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

int main (int	arc, char	*argv[]) {
	INIT_LOGGER ("ACCdemo.log_prop");

	//# Need at least one argument.
	if (argc != 1) {
		LOG_FATAL (formatString("Illegal invocation. Syntax: %s parameterfile", 
					basename(argv[0])));0
		return 1;
	}

	//# First create all the parameter files.
	for (int processNr = 0; processNr < ...; processNr--) {
		//# Read in the parameterfile.
		ParameterSet		theParams(argv[1]);

		//# Merge with process and state dependencies
		theParams.adoptFile(argv[2]);
		theParams.adoptFile(argv[3]);

		//# Write the result
		theParams.writeFile(argv[4]);

		//# register the process in our admin
		registerProcess(execName, fileName, port);
	}

	//# Open TCP/IP channel with MAC
	... cmdPort = listen(getInt("command.port")) ...

	//# Enter main loop: wait for instructions from MAC
	//# and execute the corresponding sequences.
	bool		running = true;
	bool		healthy = true;
	while (running) {
		//# The MAC command included the schedule for the events
		//# Wait till next eventtime is reached or MAC overrules
		//# the schedules by sending a command.
		Time	nextEventTime = ApplSchedule.nextEventTime(curtime);
		Event	curEvent      = ApplSchedule.getEvent(nextEventTime);

		Event	newEvent.receive(cmdPort, nextEventTime);	//# timeout or overrule
		if (curtime < nextEventTime) {					//# received MAC cmd
			if (newEvent.time() < curEvent.time()) {	//# first Cmd to Execute?
				curEvent = newEvent;					//# Adopt as first
			}
			else {
				ApplSchedule.addEvent(newEvent);		//# Just schedule
			}
			continue;
		}

		switch (curEvent.eventCode()) {
		case MCS_PREPARE: {							//# construct and prepare
			ProcessIterator		PI = theProcs.begin();
			errors = false;
			while (PI != theProcs.end()) {
				if (!PI->isRunning() && !PI->isBooting()) {
					string 	command = PI->execName() + PI->paramFile();
					int result = system (command._cstr());
					if (result == -1) {
						... oops an error ...
						error = true;
					}
					PI->booting(true);
					CmdChannels.addClientPort(PI->commandPort());
				}
				*PI++;								//# hop to next process
			}

			//# Now all processes are fired up we should be able to connect
			//# to them.
			int		retry = 0
			bool	allChannelsUp = false;
			while (!allChannelsUp && retry < MAX_FIREUP_RETRY) {
				bool failure = false;
				PI 		= theProcs.begin();
				while (PI != theProcs.end()) {
					//# if not yet connected but successfull connection is made
					if (!PI->isRunning() && CmdChannels.try2Connect(PI->portNr()) {
						theProces.running(true);
					}
					*PI++;							//# try next channel
				}
				AllChannelsUp = !failure;
				if (!allChannelsUp) {
					sleep (CHOKE_TIMEOUT);
				}
			}

			if (!AllChannelsUp) {
				... report problems to MAC ...
			}
		} 
		break;

		case ACE_START:								//# execute main task
		case ACE_STOP:								//# abort main task
		case ACE_RESUME:
		case ACE_SNAPSHOT:
		case ACE_RECOVER:
		case ACE_RECONFIGURE:
			theProcs.sendAll(curEvent.eventCode());
			break;

		default:
			LOG_INFO (formatString("Eventcode %d received. Aborting.", 
														newEvent.eventCode()));
			THROW (Exception, "Received an unknown event from the Mac Layer. Aborting.");
		} //# switch

	} //# while
		
	//# Nicely close our connections.
	... TCPclose ...

	return (0);
}

} // namespace LOFAR
