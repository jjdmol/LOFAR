//#  ProcessController.cc: Implements a control framework for ACC applications
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
//#
//#  This is a prototype of the process controller that may be used in the
//#	 ACC project. It is not intended to be complete (esp. for error checking)
//#	 but it should be usefull in getting the idea behind the process controller
//#  concept.

#include <ACC/ProcessState.h>
#include <ACC/Event.h>
#include <ACC/ACCEvents.h>
#include <ACC/Socket.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

namespace LOFAR {
extern	void main_init(ProcessState&	state);
}

int main (int	argc, char	*argv[]) {
	INIT_LOGGER ("ACCdemo.log_prop");

	//# Need at least one argument.
	if (argc != 2) {
		LOG_FATAL (formatString("Illegal invocation. Syntax: %s parameterfile", 
					basename(argv[0])));
		return 1;
	}

	//# Read in the parameterfile.
	ParameterSet		theParams(argv[1]);

	//# Get my identity and construct subset with my parameters
	string				myName   = theParams.getString("process_controller");
	ParameterSet		myParams = theParams.makeSubset(myName+".");

	//# We should fork so the AC can continue.
	LOG_TRACE_FLOW("Forking...");
	if (fork() != 0) {		//# child gets 0, parent gets -1 or cpid
		return (0);			//# parent returns always, also on failure
	}
	
	//# Open TCP/IP channel for Application Controller
	Socket		listenSocket;
	if (!listenSocket.openListener(myParams.getInt("command.port"))) {
		LOG_FATAL ("Cannot open communication with application controller");
		return (1);
	}

	//# Get the functions of this executable by calling main_init, the
	//# only 'hard' call to the executable code.
	ProcessState		state;
	main_init(state);

	//# wait for AC to connect
	LOG_TRACE_FLOW("Waiting for connection from AC");
	Socket		dataSocket;
	do {
		dataSocket = listenSocket.accept();	
	} while (!(dataSocket.isConnected()));
	LOG_TRACE_FLOW("AC opened a connection with me.");

	//# Enter main loop: wait for instructions from application controller
	//# and execute the corresponding sequences.
	bool		running = true;
	bool		healthy = true;
	while (running) {
		//# wait blocking for an event;
		LOG_TRACE_FLOW("Waiting for events.");
		char*	msgPtr;
		int32	msgLen;
		//# -1: do a blocking poll
		int32	msgSize = dataSocket.poll(&msgPtr, &msgLen, -1);

		//# Check for disconnect event
	    if (msgSize < 0) {
			LOG_TRACE_FLOW("Connection with ACC closed, quiting");
			running = false;
			break;
		}
		
		//# NOTE: the events send from the AC are 'parameterfiles' containing
		//# all the key-value pairs of the Event-data members.
		//# This satifies the needs for this demo (and simplifies debugging).
		//# In the real version a more comprehensive method can be used.
		LOG_TRACE_FLOW("Received some data, converting it to an event");
		Event	newEvent = Event(string(msgPtr));	//# wait for instructions
	
		//# The sequences in the switch below are just an example. The real
		//# sequences have to be defined still.
		switch (newEvent.eventType()) {
		case ACE_PREPARE:							//# construct and prepare
			state.checkPS(theParams);
			state.prepare(theParams);
			healthy = state.sayt();
			break;

		case ACE_START:								//# execute main task
			state.execute(myParams.getInt("runmode.nrOfCycles"));
			break;

		case ACE_STOP:								//# abort main task
			state.halt();
			break;

		case ACE_RESUME:
			healthy = state.sayt();
			if (healthy) {
				state.execute(myParams.getInt("runmode.nrOfCycles"));
			}
			break;

		case ACE_SNAPSHOT:
			state.halt();
			state.saveState();
			healthy = state.sayt();
			break;

		case ACE_RECOVER:
			state.halt();
			state.loadState();
			healthy = state.sayt();
			break;

		case ACE_RECONFIGURE:
			state.halt();
			state.quit();
			state.checkPS(theParams);
			state.prepare(theParams);
			healthy = state.sayt();
			break;

		case ACE_QUIT:								//# abort main task
			state.quit();
			running = false;
			break;

		default:
			LOG_INFO (formatString("Eventcode %d received. Aborting.", 
														newEvent.eventType()));
			THROW (Exception, "Received an unknown event from the Application Controller. Aborting.");
		} //# switch

		//# TODO: should we report our status back to the AC?
		//...reportStatus(healthy);

	} //# while
		
	//# Tell executable we are going down.
	state.quit();

	//# Nicely close our connections.
	dataSocket.disconnect();
	listenSocket.disconnect();

	return (0);
}

