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
#include <GCF/GCF_PVBool.h>
#include <GCF/GCF_PVChar.h>
#include <Common/lofar_iostream.h>
#include "Echo_Protocol.ph"

Echo::Echo(string name) : GCFTask((State)&Echo::initial, name)
{
  // register the protocol for debugging purposes
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_signalnames);

  // initialize the port
  server.init(*this, "server", GCFPortInterface::SPP, ECHO_PROTOCOL);
}

Echo::~Echo()
{
  cout << "Deleting (SAL)echoapp" << endl;
}

GCFEvent::TResult Echo::initial(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_INIT:
    {
      break;
    }
    case F_ENTRY:
      server.open();
      break;

    case F_CONNECTED:
    {
      TRAN(Echo::connected);
      break;
    }
    case F_DISCONNECTED:
      server.setTimer(1.0); // try again after 1 second
      break;

    case F_TIMER:
      server.open(); // try again
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult Echo::connected(GCFEvent& e, GCFPortInterface& /*p*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static string propName("Test_Prop");

  switch (e.signal)
  {
    case F_DISCONNECTED:
      service.dpDelete(propName);
      service.dpDelete(propName + "_test");
      cout << "Lost connection to client" << endl;
      GCFTask::stop();
      break;

    case ECHO_PING:
    {
      EchoPingEvent ping(e);

      switch (ping.seqnr % 13)
      {
        case 0:
          service.dpCreate(propName, "LPT_BOOL");
          break;
        case 1:
          service.dpCreate(propName + "_test", "LPT_CHAR");
          break;
        case 2:
          service.dpeSubscribe(propName);
          break;
        case 3:
          service.dpeSubscribe(propName + "_test");
          break;
        case 4:
          service.dpeGet(propName);
          break;
        case 5:
        {
          GCFPVBool wrongTestVal(true);
          service.dpeSet(propName + "_test", wrongTestVal);
          GCFPVChar goodTestVal('A');
          service.dpeSet(propName + "_test", goodTestVal);
          break;
        }
        case 6:
          service.dpeUnsubscribe(propName + "_test1");
          service.dpeUnsubscribe(propName + "_test");
          service.dpeUnsubscribe(propName);
          break;
        case 7:
        {
          GCFPVChar testVal('B');
          service.dpeSet(propName + "_test", testVal);
          break;
        }
        case 8:
        {
          service.dpeSubscribe(propName);
          service.dpeUnsubscribe(propName);
          GCFPVBool testVal(true);
          service.dpeSet(propName, testVal);
          break;
        }
        case 9:
          service.dpeSubscribe(propName);
          break;
        case 10:
          service.dpeGet(propName);
          service.dpeUnsubscribe(propName);
          break;
        case 11:
        {
          service.dpeSubscribe(propName);
          service.dpeUnsubscribe(propName);
          service.dpeSubscribe(propName);
          GCFPVBool testVal(false);
          service.dpeSet(propName, testVal);
          break;
        }
        case 12:
          service.dpDelete(propName);
          service.dpDelete(propName + "_test");
          break;
      }
      
      
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
