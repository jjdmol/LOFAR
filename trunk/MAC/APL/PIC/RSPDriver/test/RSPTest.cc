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
#include "EPA_Protocol.ph"

#include "RSPTestSuite.h"
#include "RSPTest.h"

#include "BeamletWeights.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP_Test;
using namespace std;
using namespace LOFAR;
using namespace blitz;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

#define N_BEAMLETS 256

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
      LOG_INFO_STR("sw.time=" << sw.timestamp);
      sw.rcumask.reset();
      sw.weights().resize(1, 1, N_BEAMLETS);

      sw.weights()(0, 0, Range::all()) = complex<int16>(0xdead, 0xbeaf);
	  
      sw.rcumask.set(0);

      TESTC_ABORT(m_server.send(sw), RSPTest::final);
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      TRAN(RSPTest::test002);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test002(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
      case F_ENTRY:
      {
	  START_TEST("test002", "test GETWEIGHTS");

	  /* start of the test sequence */
	  RSPGetweightsEvent sw;
	  sw.timestamp.setNow(5);
	  LOG_INFO_STR("sw.time= " << sw.timestamp);
	  sw.rcumask.reset();
	  sw.rcumask.set(0);
	  sw.cache = false;
	  
	  TESTC_ABORT(m_server.send(sw), RSPTest::final);
      }
      break;

      case RSP_GETWEIGHTSACK:
      {
	  RSPGetweightsackEvent ack(e);

	  LOG_INFO_STR("ack.weights = " << ack.weights());
	  LOG_INFO_STR("ack.time= " << ack.timestamp);

	  TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);

	  TRAN(RSPTest::test003);
      }
      break;

      case F_DISCONNECTED:
      {
	  port.close();

	  FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test003(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test003", "test SETWEIGHTS NOW!");

      /* start of the test sequence */
      RSPSetweightsEvent sw;
      sw.timestamp = Timestamp(0,0);
      LOG_INFO_STR("sw.time=" << sw.timestamp);
      sw.rcumask.reset();
      sw.weights().resize(1, 1, N_BEAMLETS);

      sw.weights()(0, 0, Range::all()) = complex<int16>(0xdead, 0xbeaf);
	  
      sw.rcumask.set(0);

      TESTC_ABORT(m_server.send(sw), RSPTest::final);
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      TRAN(RSPTest::test004);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test004(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test004", "test SETWEIGHTS multiple timesteps");

      /* start of the test sequence */
      RSPSetweightsEvent sw;

      // 20 seconds from now
      sw.timestamp.setNow(20);
      LOG_INFO_STR("sw.time=" << sw.timestamp);
      sw.rcumask.reset();

      // send weights for 10 timesteps
      sw.weights().resize(10, 1, N_BEAMLETS);

      sw.weights()(Range::all(), 0, Range::all()) = complex<int16>(0xdead, 0xbeaf);
	  
      sw.rcumask.set(0);

      TESTC_ABORT(m_server.send(sw), RSPTest::final);
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      TRAN(RSPTest::test005);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test005(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test005", "test SETSUBBANDS");

      /* start of the test sequence */
      RSPSetsubbandsEvent ss;

      ss.timestamp.setNow(5);
      ss.rcumask.reset();
      ss.rcumask.set(0);
      ss.rcumask.set(1);
      
      ss.subbands().resize(1, 10); // 10 subbands selected
      ss.subbands.nrsubbands().resize(1);

      LOG_INFO_STR("dim subbands=" << ss.subbands().dimensions());
      LOG_INFO_STR("dim nrsubband=" << ss.subbands.nrsubbands().dimensions());
      
      // set all values to 0x77
      ss.subbands() = 0x77;

      // nr of subbands = 10
      ss.subbands.nrsubbands() = 10;
      
      TESTC_ABORT(m_server.send(ss), RSPTest::final);
    }
    break;

    case RSP_SETSUBBANDSACK:
    {
      RSPSetsubbandsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      TRAN(RSPTest::test006);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test006(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test006", "test GETSTATUS");

      /* start of the test sequence */
      RSPGetstatusEvent ss;

      ss.timestamp = Timestamp(0,0);
      ss.rcumask.reset();
      ss.rcumask.set(0);
      ss.rcumask.set(1);
      
      TESTC_ABORT(m_server.send(ss), RSPTest::final);
    }
    break;

    case RSP_GETSTATUSACK:
    {
      RSPGetstatusackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("board=" << ack.sysstatus.board());
      LOG_INFO_STR("rcu="   << ack.sysstatus.rcu());
      
      TRAN(RSPTest::test007);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test007(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test007", "test GETSUBBANDS");

      RSPGetsubbandsEvent ss;

      ss.timestamp.setNow(6);
      ss.rcumask.reset(0);
      ss.rcumask.set(0);
      ss.cache = false;
      
      TESTC_ABORT(m_server.send(ss), RSPTest::final);
    }
    break;

    case RSP_GETSUBBANDSACK:
    {
      RSPGetsubbandsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("subbands=" << ack.subbands());
      LOG_INFO_STR("nsubbands=" << ack.subbands.nrsubbands());
      
      TRAN(RSPTest::test008);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test008(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test008", "test GETVERSION");

      RSPGetversionEvent gv;

      gv.timestamp = Timestamp(0,0);
      gv.cache = true;
      
      TESTC_ABORT(m_server.send(gv), RSPTest::final);
    }
    break;

    case RSP_GETVERSIONACK:
    {
      RSPGetversionackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      for (int i = 0; i < ack.versions.rsp().extent(firstDim); i++)
      {
	printf("versions[board=%d] = rsp=%d.%d, bp=%d.%d, ",
	       i,
	       ack.versions.rsp()(i) >> 4,
	       ack.versions.rsp()(i) & 0xF,
	       ack.versions.bp()(i)  >> 4,
	       ack.versions.bp()(i)  & 0xF);

	for (int j = 0; j < EPA_Protocol::N_AP; j++)
	{
	  printf("ap[%d]=%d.%d, ",
		 i * EPA_Protocol::N_AP + j,
		 ack.versions.ap()(i * EPA_Protocol::N_AP + j) >> 4,
		 ack.versions.ap()(i * EPA_Protocol::N_AP + j) &  0xF);
	}
	printf("\n");
      }
      
      TRAN(RSPTest::test009);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test009(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test009", "test SET/GET WG");

      // send wg settings for RCU 0

      RSPSetwgEvent wgset;
      wgset.timestamp.setNow();
      wgset.rcumask.reset();
      wgset.rcumask.set(0);
      wgset.settings().resize(1);
      wgset.settings()(0).freq            = 0xaabb;
      wgset.settings()(0).ampl            = 0xccdd;
      wgset.settings()(0).nof_usersamples = 0x1122;
      wgset.settings()(0).mode            = 0;
      
      TESTC_ABORT(m_server.send(wgset), RSPTest::final);
    }
    break;

    case RSP_SETWGACK:
    {
      RSPSetwgackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      // read back settings
      RSPGetwgEvent wgget;

      wgget.timestamp.setNow(1);
      wgget.rcumask.reset();
      wgget.rcumask.set(0);
      wgget.cache = false;

      TESTC_ABORT(m_server.send(wgget), RSPTest::final);
    }
    break;
    
    case RSP_GETWGACK:
    {
      RSPGetwgackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
      
      LOG_INFO_STR("freq            =" << ack.settings()(0).freq << endl << 
		   "ampl            =" << ack.settings()(0).ampl << endl <<
		   "nof_usersamples =" << ack.settings()(0).nof_usersamples << endl <<
		   "mode            =" << ack.settings()(0).mode << endl);

      TRAN(RSPTest::test010);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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

GCFEvent::TResult RSPTest::test010(GCFEvent& e, GCFPortInterface& port)
{
  static int updcount = 0;
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test010", "test UPDSTATUS");

      // subscribe to status updates
      RSPSubstatusEvent substatus;

      substatus.timestamp.setNow();
      substatus.rcumask.reset();
      substatus.rcumask.set(0);
      substatus.period = 1;
      
      TESTC_ABORT(m_server.send(substatus) > 0, RSPTest::final);
    }
    break;

    case RSP_SUBSTATUSACK:
    {
      RSPSubstatusackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
    }
    break;

    case RSP_UPDSTATUS:
    {
      RSPUpdstatusEvent upd(e);

      TESTC_ABORT(upd.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("upd.time=" << upd.timestamp);
      LOG_INFO_STR("upd.handle=" << upd.handle);
      
      LOG_INFO_STR("upd.sysstatus.board=" << upd.sysstatus.board());
      LOG_INFO_STR("upd.sysstatus.rcu(0)=" << upd.sysstatus.rcu()(0));

      if (updcount++ > 20)
      {
	RSPUnsubstatusEvent unsub;
	unsub.handle = upd.handle; // remove subscription with this handle

	TESTC_ABORT(m_server.send(unsub) > 0, RSPTest::final);
      }
    }
    break;
    
    case RSP_UNSUBSTATUSACK:
    {
      RSPUnsubstatusackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("ack.handle=" << ack.handle);

      TRAN(RSPTest::final);
      port.close();
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", RSPTest::final);
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
