//#
//#  EPAStub.cc: implementation of EPAStub class
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES

#include "EPA_Protocol.ph"

#include "RSPTestSuite.h"
#include "EPAStub.h"

#include "BeamletWeights.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP_Test;
using namespace std;
using namespace LOFAR;

EPAStub::EPAStub(string name)
    : GCFTask((State)&EPAStub::initial, name), Test(name)
{
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_signalnames);

  m_server.init(*this, "client", GCFPortInterface::SPP, EPA_PROTOCOL);
}

EPAStub::~EPAStub()
{}

GCFEvent::TResult EPAStub::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
      case F_INIT:
      {
      }
      break;

      case F_ENTRY:
      {
	  m_server.open();
      }
      break;

      case F_CONNECTED:
      {
	  TRAN(EPAStub::connected);
      }
      break;

      case F_DISCONNECTED:
      {
	  port.setTimer((long)1);
	  port.close();
      }
      break;

      case F_TIMER:
      {
	  // try again
	  m_server.open();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult EPAStub::connected(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
  case F_ENTRY:
      {
	  START_TEST("connected", "The connected state of the EPAStub");
      }
      break;


    case EPA_BFCOEFS:
      // simply ignore
      break;

    case EPA_RSPSTATUS:
      status = rspstatus(event, port);
      break;
      
  case F_DISCONNECTED:
      {
	  port.close();

	  TRAN(EPAStub::initial);
      }
      break;

  case F_EXIT:
      {
	  STOP_TEST();
      }
      break;

  default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EPAStub::final(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_ENTRY:
      GCFTask::stop();
      break;

    case F_EXIT:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EPAStub::rspstatus(GCFEvent& event, GCFPortInterface& port)
{
  EPARspstatusEvent rspstatus(event);

  // set the correct header info
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READRES);

  // simply echo the status event
  port.send(rspstatus);

  return GCFEvent::HANDLED;
}

void EPAStub::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  char *name = 0;
  
  GCFTask::init(argc, argv);

  for (int arg = 0; arg < argc; arg++)
  {
    if (!strcmp(argv[arg], "-name"))
    {
      if (arg++ < argc) name = argv[arg];
    }
  }

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("EPA Firmware Stub", &cerr);
  s.addTest(new EPAStub((name?name:"EPAStub")));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
