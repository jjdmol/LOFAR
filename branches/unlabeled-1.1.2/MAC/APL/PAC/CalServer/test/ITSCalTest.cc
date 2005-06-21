//#
//#  ITSCalTest.cc: implementation of ITSCalTest class
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
#include "CAL_Protocol.ph"

#include "TestSuite.h"
#include "ITSCalTest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace CAL_Protocol;

ITSCalTest::ITSCalTest(string name)
  : GCFTask((State)&ITSCalTest::initial, name), Test(name), m_handle(0), m_counter1(0)
{
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, CAL_PROTOCOL);
}

ITSCalTest::~ITSCalTest()
{}

GCFEvent::TResult ITSCalTest::initial(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(ITSCalTest::test001);
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

GCFEvent::TResult ITSCalTest::test001(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //
  // Scenario for test001 (successful start, subscribe, update, unsubscribe, stop)
  // 
  // START       ->
  //             <- STARTACK
  // SUBSCRIBE   ->
  //             <- SUBSCRIBEACK
  //
  //             <- UPDATE (*) (1st update)
  //
  //             <- UPDATE (*) (2nd update)
  // UNSUBSCRIBE ->
  //             <- UNSUBSCRIBEACK
  // STOP        ->
  //             <- STOPACK
  //
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	ostringstream name;
	name << "test001_pid=" << (int)getpid();
	m_name = name.str();
	START_TEST(m_name, "test START");

	CALStartEvent start;

	start.name   = m_name;
	start.parent = "ITS-LBA";
	start.subset.reset();

	// first 30 antennas (60 receivers)
	for (int i = 0; i < 60; i++) {
	  start.subset.set(i);
	}
	start.sampling_frequency = 40e6; // 160 MHz
	start.nyquist_zone = 2;

	TESTC_ABORT(m_server.send(start), ITSCalTest::final);
      }
      break;

    case CAL_STARTACK:
      {
	CALStartackEvent ack(e);

	TESTC_ABORT(ack.name == m_name, ITSCalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, ITSCalTest::final);

	// send subscribe
	CALSubscribeEvent subscribe;

	subscribe.name = m_name;
	subscribe.subbandset.reset();
	subscribe.subbandset.set(100);

	TESTC_ABORT(m_server.send(subscribe), ITSCalTest::final);
      }
      break;

    case CAL_SUBSCRIBEACK:
      {
	CALSubscribeackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, ITSCalTest::final);
	m_handle = ack.handle;
      }
      break;
      
    case CAL_UPDATE:
      {
	CALUpdateEvent update(e);

	LOG_INFO_STR("CAL_UPDATE @ " << update.timestamp);
	TESTC_ABORT(update.status == SUCCESS, ITSCalTest::final);
	TESTC_ABORT(update.handle == m_handle, ITSCalTest::final);
	
	LOG_INFO_STR("gains.shape = " << update.gains.getGains().shape());
	LOG_INFO_STR("quality.shape = " << update.gains.getQuality().shape());

	LOG_INFO_STR("gains=" << update.gains.getGains());

	CALUnsubscribeEvent unsubscribe;
	
	unsubscribe.handle = m_handle;
	
	TESTC_ABORT(m_server.send(unsubscribe), ITSCalTest::final);
      }
      break;

    case CAL_UNSUBSCRIBEACK:
      {
	CALUnsubscribeackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, ITSCalTest::final);
	TESTC_ABORT(ack.handle == m_handle, ITSCalTest::final);

	m_handle = 0; // clear handle

	CALStopEvent stop;
	stop.name = m_name;
	TESTC_ABORT(m_server.send(stop), ITSCalTest::final);
      }
      break;

    case CAL_STOPACK:
      {
	CALStopackEvent ack(e);
	TESTC_ABORT(ack.name == m_name, ITSCalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, ITSCalTest::final);

	TRAN(ITSCalTest::final); // next test
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();

	FAIL_ABORT("unexpected disconnect", ITSCalTest::final);
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

GCFEvent::TResult ITSCalTest::final(GCFEvent& e, GCFPortInterface& /*port*/)
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

void ITSCalTest::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("RSPDriver Test driver", &cerr);
  s.addTest(new ITSCalTest("ITSCalTest"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
