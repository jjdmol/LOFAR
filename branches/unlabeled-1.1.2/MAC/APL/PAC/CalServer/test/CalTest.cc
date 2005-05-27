//#
//#  CALTest.cc: implementation of CALTest class
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
#include "CalTest.h"

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
using namespace RTC;
using namespace CAL_Protocol;
using namespace CAL_Test;

CalTest::CalTest(string name)
  : GCFTask((State)&CalTest::initial, name), Test(name), m_handle(0)
{
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, CAL_PROTOCOL);
}

CalTest::~CalTest()
{}

GCFEvent::TResult CalTest::initial(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(CalTest::test001);
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

GCFEvent::TResult CalTest::test001(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //
  // Scenario for test001 (successful start, subscribe, update, unsubscribe, stop)
  // 
  // START       ->
  //             <- STARTACK
  // SUBSCRIBE   ->
  //             <- SUBSCRIBEACK
  // (timer 10 sec)
  //             <- UPDATE (*)
  // UNSUBSCRIBE ->
  //             <- UNSUBSCRIBEACK
  // STOP        ->
  //             <- STOPACK
  //
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	START_TEST("test001", "test START");

	CALStartEvent start;

	start.name   = "test001";
	start.parent = "LBA";
	start.subset.reset();

	// first 10 antennas (20 receivers)
	for (int i = 0; i < 20; i++) {
	  start.subset.set(i);
	}
	start.sampling_frequency = 160e6; // 160 MHz
	start.nyquist_zone = 1;

	TESTC_ABORT(m_server.send(start), CalTest::final);
      }
      break;

    case CAL_STARTACK:
      {
	CALStartackEvent ack(e);

	TESTC_ABORT(ack.name == "test001", CalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);

	// send subscribe
	CALSubscribeEvent subscribe;

	subscribe.name = "test001";
	subscribe.subbandset.reset();
	subscribe.subbandset.set(100);

	TESTC_ABORT(m_server.send(subscribe), CalTest::final);
      }
      break;

    case CAL_SUBSCRIBEACK:
      {
	CALSubscribeackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);
	m_handle = ack.handle;

	// start timer
	m_server.setTimer(4.0);
      }
      break;
      
    case CAL_UPDATE:
      {
	CALUpdateEvent update(e);

	LOG_INFO_STR("CAL_UPDATE @ " << update.timestamp);
	TESTC_ABORT(update.status == SUCCESS, CalTest::final);
	TESTC_ABORT(update.handle == m_handle, CalTest::final);
	
	LOG_INFO_STR("gains.shape = " << update.gains.getGains().shape());
	LOG_INFO_STR("quality.shape = " << update.gains.getQuality().shape());
      }
      break;

    case F_TIMER:
      {
	CALUnsubscribeEvent unsubscribe;
	
	unsubscribe.handle = m_handle;

	TESTC_ABORT(m_server.send(unsubscribe), CalTest::final);
      }
      break;

    case CAL_UNSUBSCRIBEACK:
      {
	CALUnsubscribeackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);
	TESTC_ABORT(ack.handle == m_handle, CalTest::final);

	m_handle = 0; // clear handle

	CALStopEvent stop;
	stop.name = "test002";
	TESTC_ABORT(m_server.send(stop), CalTest::final);
      }
      break;

    case CAL_STOPACK:
      {
	CALStopackEvent ack(e);
	TESTC_ABORT(ack.name == "test002", CalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);

	TRAN(CalTest::test002);
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();

	FAIL_ABORT("unexpected disconnect", CalTest::final);
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

GCFEvent::TResult CalTest::test002(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //
  // Scenario for test002 (successful start, subscribe, update, stop [no unsubscribe])
  // 
  // START       ->
  //             <- STARTACK
  // SUBSCRIBE   ->
  //             <- SUBSCRIBEACK
  //             <- UPDATE (*)
  // (timer 10 sec)
  // STOP        ->
  //             <- STOPACK
  //
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	START_TEST("test002", "test START");

	CALStartEvent start;

	start.name   = "test002";
	start.parent = "LBA";
	start.subset.reset();

	// first 10 antennas (20 receivers)
	for (int i = 0; i < 20; i++) {
	  start.subset.set(i);
	}
	start.sampling_frequency = 160e6; // 160 MHz
	start.nyquist_zone = 1;

	TESTC_ABORT(m_server.send(start), CalTest::final);
      }
      break;

    case CAL_STARTACK:
      {
	CALStartackEvent ack(e);

	TESTC_ABORT(ack.name == "test002", CalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);

	// send subscribe
	CALSubscribeEvent subscribe;

	subscribe.name = "test002";
	subscribe.subbandset.reset();
	subscribe.subbandset.set(100);

	TESTC_ABORT(m_server.send(subscribe), CalTest::final);
      }
      break;

    case CAL_SUBSCRIBEACK:
      {
	CALSubscribeackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);
	m_handle = ack.handle;

	// start timer
	m_server.setTimer(4.0);
      }
      break;

    case CAL_UPDATE:
      {
	CALUpdateEvent update(e);

	LOG_INFO_STR("CAL_UPDATE @ " << update.timestamp);
	TESTC_ABORT(update.status == SUCCESS, CalTest::final);
	TESTC_ABORT(update.handle == m_handle, CalTest::final);
	
	LOG_INFO_STR("gains.shape = " << update.gains.getGains().shape());
	LOG_INFO_STR("quality.shape = " << update.gains.getQuality().shape());
      }
      break;

    case F_TIMER:
      {
	CALStopEvent stop;
	stop.name = "test002";
	TESTC_ABORT(m_server.send(stop), CalTest::final);
      }
      break;

    case CAL_STOPACK:
      {
	CALStopackEvent ack(e);
	TESTC_ABORT(ack.name == "test002", CalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);

	m_handle = 0;

	TRAN(CalTest::final);
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();

	FAIL_ABORT("unexpected disconnect", CalTest::final);
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

GCFEvent::TResult CalTest::final(GCFEvent& e, GCFPortInterface& /*port*/)
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

void CalTest::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("RSPDriver Test driver", &cerr);
  s.addTest(new CalTest("CalTest"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
