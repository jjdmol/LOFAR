//#
//#  RSPTest.cc: implementation of RSPTest class
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

#include "RSP_Protocol.ph"

#include "RSPTestSuite.h"
#include "RSPTest.h"

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

RSPTest::RSPTest(string name)
    : GCFTask((State)&RSPTest::initial, name), Test(name)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

RSPTest::~RSPTest()
{}

GCFEvent::TResult RSPTest::initial(GCFEvent& e, GCFPortInterface& port)
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
	  TRAN(RSPTest::test001);
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

GCFEvent::TResult RSPTest::test001(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
      case F_ENTRY:
      {
	  START_TEST("test001", "test SETWEIGHTS");

	  /* start of the test sequence */
	  RSPSetweightsEvent sw;
	  sw.timestamp.setNow(10);
	  sw.rcumask.reset();
	  sw.rcumask.set(10);
	  sw.weights.weights().resize(1, sw.rcumask.count(), 10);

	  TESTC_ABORT(m_server.send(sw), RSPTest::final);
      }
      break;

      case RSP_SETWEIGHTSACK:
      {
	  RSPSetweightsackEvent swack(e);

	  TESTC_ABORT(swack.status != SUCCESS, RSPTest::final);
      }
      break;

      case F_DISCONNECTED:
      {
	  port.setTimer((long)1);
	  port.close();

	  TRAN(RSPTest::final);
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

GCFEvent::TResult RSPTest::final(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
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

void RSPTest::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("RSPDriver Test driver", &cerr);
  s.addTest(new RSPTest("RSPTest"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
