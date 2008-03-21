//#
//#  EPATest.cc: implementation of EPATest class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <MACIO/MACServiceInfo.h>

#include <Suite/suite.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <APL/BS_Protocol/BS_Protocol.ph>

#include "EPATest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <time.h>

using namespace LOFAR;
using namespace BS;
using namespace std;
using namespace EPA_Protocol;

EPATest::EPATest(string name, char* subarrayname, int startbeamlet, int nbeamlets)
  : GCFTask((State)&EPATest::initial, name), Test("EPATest"), m_subarrayname(subarrayname), m_startbeamlet(startbeamlet), m_nbeamlets(nbeamlets)
{
  registerProtocol(BS_PROTOCOL, BS_PROTOCOL_STRINGS);

  beam_server.init(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::SAP, BS_PROTOCOL);
}

EPATest::~EPATest()
{}

GCFEvent::TResult EPATest::initial(GCFEvent& e, GCFPortInterface& port)
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
	  TRAN(EPATest::test001);
      }
      break;

      case F_DISCONNECTED:
      {
	  // only do 5 reconnects
	  if (disconnect_count++ > 5)
	  {
	      FAIL("timeout");
	      TRAN(EPATest::done);
	  }
	  port.setTimer((long)2);
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

GCFEvent::TResult EPATest::test001(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;
  static uint32 beam_handle = 0;
  
  switch (e.signal)
  {
      case F_ENTRY:
      {
	LOG_INFO("running test001");

	// send beam allocation, select all subbands
	BSBeamallocEvent alloc;
	alloc.name = "Beam1";
	alloc.subarrayname = m_subarrayname;
	for (int i = m_startbeamlet; i < m_startbeamlet + m_nbeamlets; i++)
	{
	    alloc.allocation()[i] = i;
	}

	TESTC(beam_server.send(alloc));
      }
      break;

      case BS_BEAMALLOCACK:
      {
	BSBeamallocackEvent ack(e);

	if (BS_Protocol::SUCCESS != ack.status) {
	  FAIL("failed to allocate beam");
	  TRAN(EPATest::done);
	}

	beam_handle = ack.handle;
	LOG_DEBUG(formatString("got beam_handle=%d", beam_handle));

	// send pointto command (zenith)
	BSBeampointtoEvent pointto;
	pointto.handle = ack.handle;

	for (int t = 0; t <= 4; t++)
	{
	    switch (t) {
	    case 0:
	      //3C461 Cassiopeia A
	      pointto.pointing = Pointing(6.123662, 1.026719, RTC::Timestamp::now(t*30+10), Pointing::J2000);
	      break;

	    case 1:
	      // 3C405 Cygnus A
	      pointto.pointing = Pointing(5.233748, 0.711018, RTC::Timestamp::now(t*30+10), Pointing::J2000);
	      break;

	    case 2:
	      // 3C144 Crab nebula (NGC 1952)
	      pointto.pointing = Pointing(1.459568, 0.384089, RTC::Timestamp::now(t*30+10), Pointing::J2000);
	      break;

	    case 3:
	      // 3C274 Virgo NGC4486(M87)
	      pointto.pointing = Pointing(3.276114, 0.216275, RTC::Timestamp::now(t*30+10), Pointing::J2000);
	      break;
	    default:
	      break;
	    }

	    TESTC(beam_server.send(pointto));
	}

	// let the beamformer compute for 1 minute
	//timerid = beam_server.setTimer((long)60);
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
	TRAN(EPATest::done);
      }
      break;

      case F_DISCONNECTED:
      {
        FAIL("disconnected");
	port.close();
	TRAN(EPATest::done);
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

GCFEvent::TResult EPATest::done(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
  case F_ENTRY:
    GCFTask::stop();
    break;
  }

  return status;
}

void EPATest::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  if (argc != 4) {
    cerr << "usage: EPATest subarrayname startbeamlet nbeamlets" << endl;
    cerr << "e.g. EPATest FTS-1-LBA 512" << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("Beam Server Process Test Suite", &cerr);
  s.addTest(new EPATest("EPATest", argv[1], atoi(argv[2]), atoi(argv[3])));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
