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
#include "Echo_Protocol.ph"
#include <Common/lofar_iostream.h>


Echo::Echo(string name) : GCFTask((State)&Echo::initial, name)
{
  // register the protocol for debugging purposes
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_signalnames);

  // initialize the port
  server.init(*this, "server", GCFPortInterface::SPP, ECHO_PROTOCOL);
  spidDriver.init(*this, "spid", GCFPortInterface::SPP, 0, true);
}

GCFEvent::TResult Echo::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_INIT:
      break;

    case F_ENTRY:
    case F_TIMER:
      if (!server.isConnected())
        server.open();
      if (!spidDriver.isConnected())
        spidDriver.open();
      break;

    case F_CONNECTED:
      if (server.isConnected() && spidDriver.isConnected())
      {
        TRAN(Echo::connected);
      }
      break;

    case F_DISCONNECTED:
      if (!server.isConnected())
        server.setTimer(1.0); // try again after 1 second
      else
        spidDriver.setTimer(1.0);
      break;


    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult Echo::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      cout << "Lost connection to client" << endl;
      TRAN(Echo::initial);
      break;

    case ECHO_PING:
    {
      EchoPingEvent ping(e);
      // for instance these 3 lines can force an interrupt on the parallele port if
      // the pin 9 and 10 are connected together. The interrupt can be seen by means of
      // the F_DATAIN signal (see below)
      EchoClockEvent c;
      c.clockpulse = 0xFF;
      //spidDriver.send(c);
      cout << "PING received (seqnr=" << ping.seqnr << ")" << endl;
      
      timeval echo_time;
      gettimeofday(&echo_time, 0);
      EchoEchoEvent echo;
      echo.seqnr = ping.seqnr;
      echo.ping_time = ping.ping_time;
      echo.echo_time = echo_time;
      
      server.send(echo);
      
      cout << "ECHO sent" << endl;
      break;
    }
    
    case F_DATAIN:
    {
      cout << "Clock pulse: ";
      // always the recv has to be invoked. Otherwise this F_DATAIN keeps comming 
      // on each select
      char pulse[4096]; // size never mind
      p.recv(pulse, 4096); // will always return 1 if an interrupt was occured and 0 if not
      pulse[1] = 0; // if interrupt occured the first char is filled with a 'p' by the driver
      cout << pulse << endl;
      break;
    }
      
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

