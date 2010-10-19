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

//#define EARLY_REPLY

#include "EPA_Protocol.ph"
#include "RawEvent.h"

#include "EPAStub.h"
#include "RSPTestSuite.h"

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

  m_server.init(*this, "client", GCFPortInterface::SPP, EPA_PROTOCOL, true /*raw*/);
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

    case F_DATAIN:
    {
      status = RawEvent::dispatch(*this, port);
    }
    break;

    case EPA_WGSETTINGS:
    case EPA_WGUSER:
    case EPA_NRSUBBANDS:
    case EPA_SUBBANDSELECT:
    case EPA_BFCOEFS:
    case EPA_STSTATS:
    case EPA_RCUSETTINGS:
    {
      // ignore write commands, a RspstatusreadEvent will follow
    }
    break;

    case EPA_FWVERSION_READ:
      status = fwversion(event, port);
      break;

    case EPA_RSPSTATUS_READ:
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

GCFEvent::TResult EPAStub::fwversion(GCFEvent& /*event*/, GCFPortInterface& port)
{
  EPAFwversionEvent version;

  // set the correct header info
  MEP_FWVERSION(version.hdr, MEPHeader::READRES);
  version.rsp_version = (1 << 4) & 2; // version 1.2
  version.bp_version = (3 << 4) & 4;  // version 2.4
  
  for (int i = 0; i < EPA_Protocol::N_AP; i++)
  {
    version.ap_version[i] = (5 << 4) & 6; // version 5.6
  }
  
  port.send(version);

  return GCFEvent::HANDLED;
}

GCFEvent::TResult EPAStub::rspstatus(GCFEvent& /*event*/, GCFPortInterface& port)
{
  EPARspstatusEvent rspstatus;

  // set the correct header info
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READRES);
  memset(&rspstatus.board, 0, MEPHeader::RSPSTATUS_SIZE);

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
