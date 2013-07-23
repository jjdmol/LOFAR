//
//  tPythonClient.cc: Test program to test all kind of usage of the GCF ports.
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
//  $Id: tPythonClient.cc 13125 2009-04-19 12:32:55Z overeem $
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <math.h>
#include <GCF/TM/GCF_Scheduler.h>
#include "tPythonClient.h"
#include "PythonTest_Protocol.ph"

namespace LOFAR {
 namespace GCF {
  namespace TM {


// Constructors of both classes
tPythonClient::tPythonClient(string name, uint	portNr) : 
	GCFTask         ((State)&tPythonClient::connect2Server, name),
	itsTimerPort    (0),
	itsDataPort		(0),
	itsPortNr		(portNr)
{ 
}

tPythonClient::~tPythonClient()
{
	delete itsTimerPort;
	delete itsDataPort;
}

//
// connect2Server
//
// We simply set one timer in different ways and wait for the timer to expire.
//
GCFEvent::TResult tPythonClient::connect2Server(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tPythonClient::connect2Server: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY:
		itsTimerPort = new GCFTimerPort(*this, "timerPort");
		ASSERTSTR(itsTimerPort, "Failed to open timerport");
		break;

	case F_INIT:
		LOG_DEBUG_STR("trying to connect 2 the server");
		itsDataPort = new GCFTCPPort(*this, "EchoServer:v1.0", GCFPortInterface::SAP, PYTHONTEST_PROTOCOL, false);
		ASSERTSTR(itsDataPort, "Cannot allocate a data port");
		itsDataPort->setPortNumber(itsPortNr);
		itsDataPort->open();
		break;

	case F_CONNECTED: 
		LOG_INFO("YES, we are connected, Sending a message within 0.5 seconds");
		itsTimerPort->setTimer(0.5);
		break;

	case F_TIMER:
		TRAN(tPythonClient::sendPing);
		break;

	case F_DISCONNECTED: 
		LOG_INFO("oops, the connection failed");
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in connect2Server");
		break;

	default:
		LOG_WARN_STR("DEFAULT in connect2Server: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

//
// sendPing
//
// Send a Ping command and wait for the response
//
GCFEvent::TResult tPythonClient::sendPing(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	LOG_DEBUG_STR ("tPythonClient::sendPing: " << eventName(event.signal));

	switch (event.signal) {
	case F_ENTRY: {
		TEST_PTCIntegersEvent	message;
		message.var_int16  = 4386;
		message.var_uint16 = 39304;
		message.var_int32  = 2003195204;
		message.var_uint32 = 2289526357;
//		message.var_int64  = 1234605616436510000L;
//		message.var_uint64 = 9833440827789220000L;
		itsDataPort->send(message);
		cout <<  message << endl;
	}
	{
		TEST_PTCFloatsEvent		message;
		message.var_float = M_PI;
		message.var_double = M_PIl;
		itsDataPort->send(message);
		cout <<  message << endl;
	}
	{
		TEST_PTCStringsEvent		message;
		message.var_string = "aap noot mies wim zus jet";
		strcpy(message.var_charArr, "gargamel");
		message.var_char = 'M';
		itsDataPort->send(message);
		cout <<  message << endl;
	}
	{
		TEST_PTCBoolsEvent		message;
		message.var_bool = true;
		itsDataPort->send(message);
		cout <<  message << endl;
		itsTimerPort->setTimer(0.1);
	} break;

	case F_TIMER:
		itsDataPort->close();
	case F_DISCONNECTED: 
		LOG_INFO("Port is closed");
		GCFScheduler::instance()->stop();
		break;

	case F_EXIT:
		LOG_INFO("F_EXIT event in sendPing");
		break;

	default:
		LOG_WARN_STR("DEFAULT in connect2Server: " << eventName(event));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}


  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

using namespace LOFAR::GCF::TM;
using namespace std;

//
// MAIN()
//
int main(int argc, char* argv[])
{
	if (argc != 2) {
		cout << "Syntax: " << argv[0] << " portnr" << endl;
		return (-1);
	}

	GCFScheduler*	theScheduler(GCFScheduler::instance());
	theScheduler->init(argc, argv);

	tPythonClient	client("clientTask", atoi(argv[1]));
	client.start(); // make initial transition

	theScheduler->run();

	return (0);
}

