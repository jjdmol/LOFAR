//#
//#  CalTest.cc: implementation of CalTest class
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
#include <APL/CAL_Protocol/CAL_Protocol.ph>

#include <MACIO/MACServiceInfo.h>

#include <APL/RTCCommon/TestSuite.h>
#include "CalTest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;
using namespace CAL_Protocol;

CalTest::CalTest(string name, string arrayname, string parentname, int nantennas, int nyquistzone, uint32 rcucontrol, int subarrayid)
  : GCFTask((State)&CalTest::initial, name), Test(name), m_handle(0), m_counter1(0),
    m_arrayname(arrayname), m_parentname(parentname), m_nantennas(nantennas), m_nyquistzone(nyquistzone), m_rcucontrol(rcucontrol), m_subarrayid(subarrayid)
{
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);

  m_server.init(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::SAP, CAL_PROTOCOL);
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
	START_TEST("test001", "test START");

	CALStartEvent start;

	start.name   = m_arrayname;
	start.parent = m_parentname;
	start.subset.reset();

	// select antennas
	for (int i = 0; i < m_nantennas; i++) {
	  switch (m_subarrayid) {
	  case 0:
	    start.subset.set(i*2);
	    start.subset.set(i*2+1);
	    break;
	  case 1:
	    // select odd antennas
	    if (1 == (i % 2)) {
	      start.subset.set(i*2);
	      start.subset.set(i*2+1);
	    }
	    break;
	  case 2:
	    // select even antennas
	    if (0 == (i % 2)) {
	      start.subset.set(i*2);
	      start.subset.set(i*2+1);
	    }
	    break;
	  }
	}
	start.rcumode().resize(1);
	start.rcumode()(0).setRaw(m_rcucontrol);

	TESTC_ABORT(m_server.send(start), CalTest::final);
      }
      break;

    case CAL_STARTACK:
      {
	CALStartackEvent ack(e);

	TESTC_ABORT(ack.name == m_arrayname, CalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);

	// send subscribe
	CALSubscribeEvent subscribe;

	subscribe.name = m_arrayname;
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

#if 0
	//LOG_INFO_STR("gains=" << update.gains.getGains());

	CALUnsubscribeEvent unsubscribe;
	
	unsubscribe.name = m_arrayname;
	unsubscribe.handle = m_handle;
	
	TESTC_ABORT(m_server.send(unsubscribe), CalTest::final);
#endif
      }
      break;

    case CAL_UNSUBSCRIBEACK:
      {
	CALUnsubscribeackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);
	TESTC_ABORT(ack.handle == m_handle, CalTest::final);

	m_handle = 0; // clear handle

	CALStopEvent stop;
	stop.name = m_arrayname;
	TESTC_ABORT(m_server.send(stop), CalTest::final);
      }
      break;

    case CAL_STOPACK:
      {
	CALStopackEvent ack(e);
	TESTC_ABORT(ack.name == m_arrayname, CalTest::final);
	TESTC_ABORT(ack.status == SUCCESS, CalTest::final);

	TRAN(CalTest::final); // next test
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

  if (argc != 8) {
    cerr << "usage: CalTest arrayname parentname nantennas nyquistzone rcucontrol subarray=[0|1|2]" << endl;
    cerr << "e.g.   CalTest FTS-1-LBA FTS-1-LBA      8        1        0x0000037A     0" << endl;
    cerr << "(see AntennaArrays.conf for other configurations)" << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("RSPDriver Test driver", &cerr);

  int rcucontrol;
  sscanf(argv[5], "%i", &rcucontrol);
  s.addTest(new CalTest("CalTest", argv[1], argv[2], int(atof(argv[3])),
			atoi(argv[4]), rcucontrol, atoi(argv[6])));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
