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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include <MACIO/MACServiceInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <APL/RTCCommon/TestSuite.h>
#include "RSPTest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace RTC;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace RSP_Test;

// number of RCUS to select in the rcumask
#define N_TEST_RCUS 8
#define N_TEST_RSPS 2

RSPTest::RSPTest(string name)
    : GCFTask((State)&RSPTest::initial, name), Test(name)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

  m_server.init(*this, MAC_SVCMASK_RSPDRIVER, GCFPortInterface::SAP, RSP_PROTOCOL);
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
      sw.rcumask.set(0);

      sw.weights().resize(1, 1, MEPHeader::N_BEAMLETS);
      sw.weights()(0, 0, Range::all()) = complex<int16>(0xdead, 0xbeaf);
	  

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
      sw.weights().resize(1, 1, MEPHeader::N_BEAMLETS);

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
      sw.weights().resize(10, 1, MEPHeader::N_BEAMLETS);

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
      ss.rcumask.set(2);
      ss.rcumask.set(3);

	  ss.subbands.setType(SubbandSelection::BEAMLET);
      
      ss.subbands().resize(1, 10); // 10 subbands selected

      LOG_INFO_STR("dim subbands=" << ss.subbands().dimensions());
      
      // set all values to 0x77
      ss.subbands() = 0x77;
      
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
      ss.rspmask.reset();
      ss.rspmask.set(0);
      ss.cache = false;
      
      TESTC_ABORT(m_server.send(ss), RSPTest::final);
    }
    break;

    case RSP_GETSTATUSACK:
    {
      RSPGetstatusackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      // << no longer supported LOG_INFO_STR("board=" << ack.sysstatus.board());
      
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
	  ss.type  = SubbandSelection::BEAMLET;
      
      TESTC_ABORT(m_server.send(ss), RSPTest::final);
    }
    break;

    case RSP_GETSUBBANDSACK:
    {
      RSPGetsubbandsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("subbands=" << ack.subbands());
      
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

#if 0
      for (int i = 0; i < ack.versions.rsp().extent(firstDim); i++)
	{
	LOG_INFO_STR(formatString("versions[board=%d] = rsp=%d.%d, bp=%d.%d, ap=%d.%d",
				  i,
				  ack.versions.rsp()(i) >> 4,
				  ack.versions.rsp()(i) & 0xF,
				  ack.versions.bp()(i)  >> 4,
				  ack.versions.bp()(i)  & 0xF,
				  ack.versions.ap()(i)  >> 4,
				  ack.versions.ap()(i)  & 0xF));
      }
#endif
 
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
      for (int i = 0; i < N_TEST_RCUS; i++) {
	wgset.rcumask.set(i);
      }
      wgset.settings().resize(1);
      wgset.settings()(0).mode            = 0;
      wgset.settings()(0).phase           = 0;
      wgset.settings()(0).nof_samples     = 1024;
      wgset.settings()(0).freq            = 0xaabbccdd;
      wgset.settings()(0).ampl            = 0x11223344;

      wgset.settings()(0).preset          = WGSettings::PRESET_SINE;
      
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
      for (int i = 0; i < N_TEST_RCUS; i++) {
	wgget.rcumask.set(i);
      }
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
		   "phase           =" << (int)ack.settings()(0).phase << endl <<
		   "ampl            =" << (int)ack.settings()(0).ampl << endl <<
		   "nof_usersamples =" << ack.settings()(0).nof_samples << endl <<
		   "mode            =" << (int)ack.settings()(0).mode << endl <<
		   "preset          =" << (int)ack.settings()(0).preset);

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
      substatus.rspmask.reset();
      substatus.rspmask.set(0);
      substatus.period = 4;
      
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
      
      // << no longer supported LOG_INFO_STR("upd.sysstatus.board=" << upd.sysstatus.board());

      if (updcount++ > 4) // four seconds
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

      TRAN(RSPTest::test011);
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

GCFEvent::TResult RSPTest::test011(GCFEvent& e, GCFPortInterface& port)
{
  static int updcount = 0;
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test011", "test GET/UPD STATS");

      // get stats (single)
      RSPGetstatsEvent getstats;

      getstats.timestamp.setNow();
      getstats.rcumask.reset();
      for (int i = 0; i < N_TEST_RCUS; i++) {
	getstats.rcumask.set(i);
      }
      getstats.cache = false;
      getstats.type = Statistics::SUBBAND_POWER;
      

      TESTC_ABORT(m_server.send(getstats) > 0, RSPTest::final);
    }
    break;

  case RSP_GETSTATSACK:
    {
      RSPGetstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time" << ack.timestamp);

      // subscribe to status updates
      RSPSubstatsEvent substats;

      substats.timestamp.setNow();
      substats.rcumask.reset();
      for (int i = 0; i < N_TEST_RCUS; i++) {
	substats.rcumask.set(i);
      }
      substats.period = 1;
      substats.type = Statistics::SUBBAND_POWER;
      substats.reduction = SUM;
      
      TESTC_ABORT(m_server.send(substats) > 0, RSPTest::final);
    }
    break;

    case RSP_SUBSTATSACK:
    {
      RSPSubstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
    }
    break;

    case RSP_UPDSTATS:
    {
      RSPUpdstatsEvent upd(e);

      TESTC_ABORT(upd.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("upd.time=" << upd.timestamp);
      LOG_INFO_STR("upd.handle=" << upd.handle);
      
      LOG_INFO_STR("upd.stats().shape()=" << upd.stats().shape());

      if (updcount++ > 4) // four seconds
      {
	RSPUnsubstatsEvent unsub;
	unsub.handle = upd.handle; // remove subscription with this handle

	TESTC_ABORT(m_server.send(unsub) > 0, RSPTest::final);
      }
    }
    break;
    
    case RSP_UNSUBSTATSACK:
    {
      RSPUnsubstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("ack.handle=" << ack.handle);

      TRAN(RSPTest::test012);
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

GCFEvent::TResult RSPTest::test012(GCFEvent& e, GCFPortInterface& port)
{
  static int updcount = 0;
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test012", "test GET/UPD XCSTATS");

      // get stats (single)
      RSPGetxcstatsEvent getxcstats;

      getxcstats.timestamp.setNow();
      getxcstats.cache = false;

      TESTC_ABORT(m_server.send(getxcstats) > 0, RSPTest::final);
    }
    break;

  case RSP_GETXCSTATSACK:
    {
      RSPGetxcstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time" << ack.timestamp);
      LOG_INFO_STR("ack.stats().shape()" << ack.stats().shape());

      // subscribe to status updates
      RSPSubxcstatsEvent subxcstats;

      subxcstats.timestamp.setNow();
      subxcstats.period = 1;
      
      TESTC_ABORT(m_server.send(subxcstats) > 0, RSPTest::final);
    }
    break;

    case RSP_SUBXCSTATSACK:
    {
      RSPSubxcstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
    }
    break;

    case RSP_UPDXCSTATS:
    {
      RSPUpdxcstatsEvent upd(e);

      TESTC_ABORT(upd.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("upd.time=" << upd.timestamp);
      LOG_INFO_STR("upd.handle=" << upd.handle);
      
      LOG_INFO_STR("upd.stats().shape()=" << upd.stats().shape());

      if (updcount++ > 4) // four seconds
      {
	RSPUnsubxcstatsEvent unsub;
	unsub.handle = upd.handle; // remove subscription with this handle

	TESTC_ABORT(m_server.send(unsub) > 0, RSPTest::final);
      }
    }
    break;
    
    case RSP_UNSUBXCSTATSACK:
    {
      RSPUnsubxcstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("ack.handle=" << ack.handle);

      TRAN(RSPTest::test013);
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

GCFEvent::TResult RSPTest::test013(GCFEvent& e, GCFPortInterface& port)
{
  static int updcount = 0;
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("test013", "test UPDSUBBANDS");

      // subscribe to SUBBANDS updates
      RSPSubsubbandsEvent subsubbands;

      subsubbands.timestamp.setNow();
      subsubbands.rcumask.reset();
      for (int i = 0; i < N_TEST_RCUS; i++) {
		subsubbands.rcumask.set(i);
      }
      subsubbands.period = 4;
	  subsubbands.type   = SubbandSelection::BEAMLET;
      
      TESTC_ABORT(m_server.send(subsubbands) > 0, RSPTest::final);
    }
    break;

    case RSP_SUBSUBBANDSACK:
    {
      RSPSubsubbandsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
    }
    break;

    case RSP_UPDSUBBANDS:
    {
      RSPUpdsubbandsEvent upd(e);

      TESTC_ABORT(upd.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("upd.time=" << upd.timestamp);
      LOG_INFO_STR("upd.handle=" << upd.handle);
      LOG_INFO_STR("upd.subbands=" << upd.subbands());

      if (updcount++ > 2) // two seconds
      {
		RSPUnsubsubbandsEvent unsub;
		unsub.handle = upd.handle; // remove subscription with this handle

		TESTC_ABORT(m_server.send(unsub) > 0, RSPTest::final);
      }
    }
    break;
    
    case RSP_UNSUBSUBBANDSACK:
    {
      RSPUnsubsubbandsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
      LOG_INFO_STR("ack.handle=" << ack.handle);

      TRAN(RSPTest::test014);
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

GCFEvent::TResult RSPTest::test014(GCFEvent& e, GCFPortInterface& port)
{
  static int updcount = 0;
  
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	START_TEST("test014", "test UPDRCU");

	// subscribe to RCU updates
	RSPSubrcuEvent subrcu;

	subrcu.timestamp.setNow();
	subrcu.rcumask.reset();
	for (int i = 0; i < N_TEST_RCUS; i++) {
	  subrcu.rcumask.set(i);
	}
	subrcu.period = 4;
      
	TESTC_ABORT(m_server.send(subrcu) > 0, RSPTest::final);
      }
      break;

    case RSP_SUBRCUACK:
      {
	RSPSubrcuackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
	LOG_INFO_STR("ack.time=" << ack.timestamp);
      }
      break;

    case RSP_UPDRCU:
      {
	RSPUpdrcuEvent upd(e);

	TESTC_ABORT(upd.status == SUCCESS, RSPTest::final);
	LOG_INFO_STR("upd.time=" << upd.timestamp);
	LOG_INFO_STR("upd.handle=" << upd.handle);
	RCUSettings::Control& x = upd.settings()(1); // REO
	LOG_INFO(formatString("upd.registers=0x%X", x.getRaw()));

	if (updcount++ > 4) // four seconds
	  {
	    RSPUnsubrcuEvent unsub;
	    unsub.handle = upd.handle; // remove subscription with this handle

	    TESTC_ABORT(m_server.send(unsub) > 0, RSPTest::final);
	  }
      }
      break;
    
    case RSP_UNSUBRCUACK:
      {
	RSPUnsubrcuackEvent ack(e);

	TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
	LOG_INFO_STR("ack.time=" << ack.timestamp);
	LOG_INFO_STR("ack.handle=" << ack.handle);

	TRAN(RSPTest::final);
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
