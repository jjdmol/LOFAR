//#
//#  AVTStub.cc: implementation of AVTStub class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <MACIO/MACServiceInfo.h>

#include <TestSuite/suite.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <APL/BS_Protocol/BS_Protocol.ph>

#include "AVTStub.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <time.h>

//
// using half of the beamlets should allow two AVTStub clients
// to connect to the BeamServer
//

using namespace LOFAR;
using namespace BS;
using namespace std;
using namespace EPA_Protocol;

AVTStub::AVTStub(string name)
  : GCFTask((State)&AVTStub::initial, name), Test("AVTStub")
{
  registerProtocol(BS_PROTOCOL, BS_PROTOCOL_STRINGS);

  beam_server.init(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::SAP, BS_PROTOCOL);
}

AVTStub::~AVTStub()
{}

GCFEvent::TResult AVTStub::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int disconnect_count = 0;

  LOG_DEBUG(formatString("initial received event on port %s", port.getName().c_str()));

  switch(e.signal)
  {
      case F_INIT:
      {
      }
      break;

      case F_ENTRY:
      {
	  beam_server.open();
      }
      break;

      case F_CONNECTED:
      {
	  LOG_DEBUG(formatString("port %s connected", port.getName().c_str()));
	  TRAN(AVTStub::test001);
      }
      break;

      case F_DISCONNECTED:
      {
	  // only do 5 reconnects
	  if (disconnect_count++ > 5)
	  {
	      FAIL("timeout");
	      TRAN(AVTStub::done);
	  }
	  port.setTimer((long)2);
	  port.close();
      }
      break;

      case F_TIMER:
      {
	  // try again
	  beam_server.open();
      }
      break;

      default:
      {
	  status = GCFEvent::NOT_HANDLED;
      }
      break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test001(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;

  switch (e.signal)
  {
      case F_ENTRY:
      {
	LOG_INFO("running test001");
  
	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// start test by sending beam alloc
	BSBeamallocEvent alloc;
	alloc.subarrayname = "ITS-LBA";
	alloc.allocation()[0] = 0;

	TESTC(beam_server.send(alloc));
      }
      break;

      case BS_BEAMALLOCACK:
      {
	BSBeamallocackEvent ack(e);

	TESTC(BS_Protocol::SUCCESS == ack.status);
	if (BS_Protocol::SUCCESS == ack.status)
	{
	  // beam allocated, now free it
	  BSBeamfreeEvent beamfree;
	  beamfree.handle = ack.handle;

	  TESTC(beam_server.send(beamfree));
	}
	else
	{
	  TRAN(AVTStub::test002);
	}
      }
      break;

      case BS_BEAMFREEACK:
      {
	BSBeamfreeackEvent ack(e);
	TESTC(BS_Protocol::SUCCESS == ack.status);

	// test completed, next test
	TRAN(AVTStub::test002);
      }
      break;

      case F_TIMER:
      {
	// abort test
	beam_server.close();
	FAIL("timeout");
	TRAN(AVTStub::done);
      }
      break;

      case F_DISCONNECTED:
      {
	FAIL("disconnected");
	port.close();
	TRAN(AVTStub::done);
      }
      break;

      case F_EXIT:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

      default:
      {
	  status = GCFEvent::NOT_HANDLED;
      }
      break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test002(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("running test002");

      // start test timer, test should finish
      // within 2 seconds
      timerid = beam_server.setTimer((long)2);

      // start test by sending beam alloc
      BSBeamallocEvent alloc;
      alloc.subarrayname = "ITS-LBA";
      alloc.allocation()[0] = 0;

      TESTC(beam_server.send(alloc));
    }
    break;

    case BS_BEAMALLOCACK:
    {
      BSBeamallocackEvent ack(e);

      TESTC(BS_Protocol::SUCCESS == ack.status);
      if (BS_Protocol::SUCCESS == ack.status)
      {
	// send pointto command
	BSBeampointtoEvent pointto;
	pointto.handle = ack.handle;
	pointto.pointing = Pointing(0.0, -1.0, RTC::Timestamp::now(15), Pointing::LOFAR_LMN);

	TESTC(beam_server.send(pointto));

	// beam pointed, now free it
	BSBeamfreeEvent beamfree;
	beamfree.handle = ack.handle;

	TESTC(beam_server.send(beamfree));
      }
      else TRAN(AVTStub::test003);
    }
    break;

    case BS_BEAMFREEACK:
    {
      BSBeamfreeackEvent ack(e);
      TESTC(BS_Protocol::SUCCESS == ack.status);

      // test completed, next test
      TRAN(AVTStub::test003);
    }
    break;

    case F_TIMER:
    {
      // abort test
      beam_server.close();
      FAIL("timeout");
      TRAN(AVTStub::done);
    }
    break;

    case F_DISCONNECTED:
    {
      FAIL("disconnected");
      port.close();
      TRAN(AVTStub::done);
    }
    break;

    case F_EXIT:
    {
      // before leaving, cancel the timer
      beam_server.cancelTimer(timerid);
    }
    break;

    default:
    {
      status = GCFEvent::NOT_HANDLED;
    }
    break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test003(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	LOG_INFO("running test003");

	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// start test by sending beam alloc
	BSBeamallocEvent alloc;
	alloc.subarrayname = "ITS-LBA";
	alloc.allocation()[0] = 0;

	TESTC(beam_server.send(alloc));
      }
      break;

    case BS_BEAMALLOCACK:
      {
	BSBeamallocackEvent ack(e);

	TESTC(BS_Protocol::SUCCESS == ack.status);
	if (BS_Protocol::SUCCESS == ack.status)
	  {
	    // send pointto command
	    BSBeampointtoEvent pointto;
	    pointto.handle = ack.handle;
	    pointto.pointing = Pointing(0.0, -1.0, RTC::Timestamp::now(15), Pointing::LOFAR_LMN);

	    TESTC(beam_server.send(pointto));

	    // beam pointed, now free it
	    BSBeamfreeEvent beamfree;
	    beamfree.handle = ack.handle;

	    TESTC(beam_server.send(beamfree));
	  }
	else TRAN(AVTStub::test004);
      }
      break;

    case BS_BEAMFREEACK:
      {
	BSBeamfreeackEvent ack(e);
	TESTC(BS_Protocol::SUCCESS == ack.status);

	// test completed, next test
	TRAN(AVTStub::test004);
      }
      break;

    case F_TIMER:
      {
	// abort test
	beam_server.close();
	FAIL("timeout");
	TRAN(AVTStub::done);
      }
      break;

    case F_DISCONNECTED:
      {
	FAIL("disconnected");
	port.close();
	TRAN(AVTStub::done);
      }
      break;

    case F_EXIT:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

    default:
      {
	status = GCFEvent::NOT_HANDLED;
      }
      break;
    }

  return status;
}

GCFEvent::TResult AVTStub::test004(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;
  static int loop = 0;
  
  switch (e.signal)
  {
      case F_ENTRY:
      {
	LOG_INFO("running test004");

	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// invalid n_subbands in beam alloc
	BSBeamallocEvent alloc;
	alloc.subarrayname = "ITS-LBA";
	alloc.allocation()[0] = 0;

	TESTC(beam_server.send(alloc));
      }
      break;

      case BS_BEAMALLOCACK:
      {
	BSBeamallocackEvent ack(e);

	TESTC(BS_Protocol::SUCCESS != ack.status);

	if (loop == 0)
	{
	  // send invalid spectral window index
	  BSBeamallocEvent alloc;
	  alloc.subarrayname = "ITS-LBA";
	  alloc.allocation()[0] = 0;

	  TESTC(beam_server.send(alloc));

	  loop++;
	}
	else if (loop == 1)
	{
	  // send invalid n_subbands (-1)
	  BSBeamallocEvent alloc;
	  alloc.subarrayname = "ITS-LBA";
	  alloc.allocation()[0] = 0;

	  TESTC(beam_server.send(alloc));

	  loop++;
	}
	else if (loop == 2)
	{
	  // send invalid index in subbands array
	  BSBeamallocEvent alloc;
	  alloc.subarrayname = "ITS-LBA";
	  alloc.allocation()[0] = 0;

	  TESTC(beam_server.send(alloc));

	  loop++;
	}
	else
	{
	    // done => next test
	    TRAN(AVTStub::test005);
	}

	LOG_DEBUG(formatString("loop=%d", loop));
      }
      break;

      case F_TIMER:
      {
	// abort test
	beam_server.close();
	FAIL("timeout");
	TRAN(AVTStub::done);
      }
      break;

      case F_DISCONNECTED:
      {
	FAIL("disconnected");
	port.close();
	TRAN(AVTStub::done);
      }
      break;

      case F_EXIT:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

      default:
      {
	  status = GCFEvent::NOT_HANDLED;
      }
      break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test005(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;
  static void* beam_handle = 0;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      LOG_INFO("running test005");

      // send beam allocation, select all subbands
      BSBeamallocEvent alloc;
      alloc.subarrayname = "ITS-LBA";
      for (int i = 0; i < MEPHeader::N_BEAMLETS; i++)
      {
	alloc.allocation()[i] = i;
      }

      TESTC(beam_server.send(alloc));
    }
    break;

    case BS_BEAMALLOCACK:
    {
      BSBeamallocackEvent ack(e);

      TESTC(BS_Protocol::SUCCESS == ack.status);
      if (BS_Protocol::SUCCESS == ack.status)
      {
	beam_handle = ack.handle;
	LOG_DEBUG(formatString("got beam_handle=%d", beam_handle));

	// send pointto command (zenith)
	BSBeampointtoEvent pointto;
	pointto.handle = ack.handle;
	pointto.pointing = Pointing(0.0, 1.0, RTC::Timestamp::now(20), Pointing::LOFAR_LMN);

	TESTC(beam_server.send(pointto));

	// send pointto command (northern horizon)
	pointto.pointing = Pointing(1.0, 0.0, RTC::Timestamp::now(25), Pointing::LOFAR_LMN);

	TESTC(beam_server.send(pointto));

	// let the beamformer compute for 30 seconds
	timerid = beam_server.setTimer((long)30);
      }
      else TRAN(AVTStub::done);
    }
    break;

    case F_TIMER:
    {
      // done => send BEAMFREE
      BSBeamfreeEvent beamfree;
      beamfree.handle = beam_handle;

      TESTC(beam_server.send(beamfree));
    }
    break;

    case BS_BEAMFREEACK:
    {
      BSBeamfreeackEvent ack(e);
      TESTC(BS_Protocol::SUCCESS == ack.status);
      TESTC(beam_handle == ack.handle);

      // test completed, next test
      TRAN(AVTStub::done);
    }
    break;

    case F_DISCONNECTED:
    {
      FAIL("disconnected");
      port.close();
      TRAN(AVTStub::done);
    }
    break;

    case F_EXIT:
    {
      // before leaving, cancel the timer
      beam_server.cancelTimer(timerid);
    }
    break;

    default:
    {
      status = GCFEvent::NOT_HANDLED;
    }
    break;
  }

  return status;
}

GCFEvent::TResult AVTStub::done(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
  case F_ENTRY:
    GCFScheduler::instance()->stop();
    break;
  }

  return status;
}

void AVTStub::run()
{
  start(); // make initial transition
  GCFScheduler::instance()->run();
}

int main(int argc, char** argv)
{
  GCFScheduler::instance()->init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("Beam Server Process Test Suite", &cerr);
  s.addTest(new AVTStub("AVTStub"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
