//
//  Echo.cc: Implementation of the Echo task class.
//
//  Copyright (C) 2003
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

#include "Echo.h"
#define DECLARE_SIGNAL_NAMES
#include "Echo_Protocol.ph"


Echo::Echo(string name) : GCFTask((State)&Echo::initial, name)
{
  // register the protocol for debugging purposes
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_signalnames);

  // initialize the port
  server.init(*this, "server", GCFPortInterface::SPP, ECHO_PROTOCOL);
}

int Echo::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_INIT_SIG:
      break;

    case F_ENTRY_SIG:
      server.open();
      break;

    case F_CONNECTED_SIG:
      TRAN(&Echo::connected);
      break;

    case F_DISCONNECTED_SIG:
      server.setTimer(1.0); // try again after 1 second
      break;

    case F_TIMER_SIG:
      server.open(); // try again
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

int Echo::connected(GCFEvent& e, GCFPortInterface& /*p*/)
{
  int status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_DISCONNECTED_SIG:
      cout << "Lost connection to client" << endl;
      TRAN(&Echo::initial);
      break;

    case ECHO_PING:
      {

#ifndef HUMAN_PORT

	EchoPingEvent* ping = static_cast<EchoPingEvent*>(&e);

	cout << "PING received (seqnr=" << ping->seqnr << ")" << endl;
	
	timeval echo_time;
	gettimeofday(&echo_time, 0);
	EchoEchoEvent echo(ping->seqnr,
			   ping->ping_time,
			   echo_time);

#else
	
	cout << "PING received" << endl;

	GCFEvent echo(ECHO_ECHO);

#endif
	server.send(echo);

	cout << "ECHO sent" << endl;
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

int main(int argc, char* argv[])
{
  GCFTask::init(argc, argv);
  
  Echo echo_task("ECHO");

  echo_task.start(); // make initial transition

  GCFTask::run();

  return 0;
}

