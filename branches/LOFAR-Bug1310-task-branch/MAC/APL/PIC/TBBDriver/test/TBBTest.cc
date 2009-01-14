//#
//#  TBBTest.cc: implementation of TBBTest class
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

#include <APL/TBB_Protocol/TBB_Protocol.ph>
#include <APL/RTCCommon/TestSuite.h>
#include "TBBTest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>


using namespace std;
using namespace LOFAR;
//using namespace RTC;
using namespace TBB_Protocol;
using namespace TBB_Test;


TBBTest::TBBTest(string name)
    : GCFTask((State)&TBBTest::initial, name), Test(name)
{
  registerProtocol (TBB_PROTOCOL,      TBB_PROTOCOL_STRINGS);

  itsClient.init(*this, MAC_SVCMASK_TBBDRIVER, GCFPortInterface::SAP, TBB_PROTOCOL);
	
	itsboardmask = 0x00000001; // bitmask with boards to test bo = board1
}

TBBTest::~TBBTest()
{}

GCFEvent::TResult TBBTest::initial(GCFEvent& e, GCFPortInterface& port)
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
      LOG_DEBUG_STR("Opening client port");
      itsClient.open();
    }
    break;

    case F_CONNECTED:
    {
      LOG_DEBUG_STR("port is connected, switch to test001");
      TRAN(TBBTest::test008);
    }
    break;

    case F_DISCONNECTED:
    {
      LOG_DEBUG_STR("port is disconnected, set timer, and close port");
      port.setTimer((long)2);
      port.close();
    }
    break;

    case F_TIMER:
    {
      // try again
      LOG_DEBUG_STR("Try to open the port again");
      itsClient.open();
    }
    break;

    default:
      LOG_DEBUG_STR("unknown message");
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult TBBTest::test001(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
			{
				START_TEST("test001", "test ALLOC");
	
				/* start of the test sequence */
				TBBAllocEvent sw;
				
				sw.rcu_mask.set();
				
				TESTC_ABORT(itsClient.send(sw), TBBTest::final);
			}
			break;

    case TBB_ALLOC_ACK:
			{
				TBBAllocAckEvent ack(e);
				LOG_INFO_STR(formatString	 ("Ack status = 0X%08X", ack.status_mask));
				for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
				LOG_INFO_STR("Alloc test OK");
	
				TRAN(TBBTest::test002);
			}
			break;

    case F_DISCONNECTED:
			{
				port.close();
	
				FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test002(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test002", "test FREE");
	
			/* start of the test sequence */
			TBBFreeEvent sw;
				
			sw.rcu_mask.set();
				
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_FREE_ACK:
		{
			TBBAllocAckEvent ack(e);
			LOG_INFO_STR(formatString	 ("Ack status = 0X%08X", ack.status_mask));
			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
			LOG_INFO_STR("Free test OK");
	
			TRAN(TBBTest::test008);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();
	
			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test003(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test003", "test RECORD");
	
			/* start of the test sequence */
			TBBRecordEvent sw;
				
			sw.rcu_mask.set();
				
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_FREE_ACK:
		{
			TBBRecordAckEvent ack(e);
	
			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
			LOG_INFO_STR("Record test OK");
	
			TRAN(TBBTest::test004);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();
	
			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test004(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test004", "test STOP");
	
			/* start of the test sequence */
			TBBStopEvent sw;
				
			sw.rcu_mask.set();
				
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_STOP_ACK:
		{
			TBBStopAckEvent ack(e);
	
			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
			LOG_INFO_STR("Stop test OK");
	
			TRAN(TBBTest::test005);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();
	
			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test005(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test005", "test TRIGCLR");
	
			/* start of the test sequence */
			TBBTrigReleaseEvent sw;

			//sw.channel = 1;
				
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_TRIG_RELEASE_ACK:
		{
			TBBTrigReleaseAckEvent ack(e);
	
			//TESTC_ABORT(ack.status_mask == TBB_SUCCESS, TBBTest::final);
			LOG_INFO_STR("TrigClr test OK");
	
			TRAN(TBBTest::test006);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();
	
			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test006(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
  
	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test006", "test READ");
	
			/* start of the test sequence */
			TBBReadEvent sw;
				
			sw.rcu = 1;
				
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_READ_ACK:
		{
			TBBReadAckEvent ack(e);
	
			TESTC_ABORT(ack.status_mask == TBB_SUCCESS, TBBTest::final);
			LOG_INFO_STR("Read test OK");
	
			TRAN(TBBTest::test007);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();
	
			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test007(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test007", "test UDP");

			/* start of the test sequence */
			TBBModeEvent sw;
			
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_MODE_ACK:
		{
			TBBModeAckEvent ack(e);

			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS; boardnr++) {
				TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
			}
			LOG_INFO_STR("Udp test OK");

			TRAN(TBBTest::test008);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();

			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test008(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test008", "test VERSION");

			/* start of the test sequence */
			TBBVersionEvent sw;
			
			sw.boardmask =itsboardmask; // boards tot test
			
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_VERSION_ACK:
		{
			TBBVersionAckEvent ack(e);
			
			for(int cnt = 0;cnt < 12;cnt++) {
				LOG_INFO_STR(formatString("boardnr      = %d", cnt));
				LOG_INFO_STR(formatString("boardid      = 0X%08X", ack.boardid[cnt]));
				LOG_INFO_STR(formatString("swversion    = 0X%08X", ack.swversion[cnt]));
				LOG_INFO_STR(formatString("boardversion = 0X%08X", ack.boardversion[cnt]));
				LOG_INFO_STR(formatString("tpversion    = 0X%08X", ack.tpversion[cnt]));
				LOG_INFO_STR(formatString("mp0version   = 0X%08X", ack.mp0version[cnt]));
				LOG_INFO_STR(formatString("mp1version   = 0X%08X", ack.mp1version[cnt]));
				LOG_INFO_STR(formatString("mp2version   = 0X%08X", ack.mp2version[cnt]));
				LOG_INFO_STR(formatString("mp3version   = 0X%08X", ack.mp3version[cnt]));
			}
			//LOG_INFO_STR(formatString	 ("Ack status = 0X%08X", ack.status_mask));
			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
			LOG_INFO_STR("Version test OK");
			
			
			TRAN(TBBTest::final);
			//TRAN(TBBTest::test009);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();

			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test009(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test009", "test SIZE");

			/* start of the test sequence */
			TBBSizeEvent sw;
			
			sw.boardmask = itsboardmask; // boards tot test
			
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_SIZE_ACK:
		{
			TBBSizeAckEvent ack(e);

			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
			LOG_INFO_STR("Size test OK");

			TRAN(TBBTest::test010);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();

			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::test010(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal)
	{
		case F_ENTRY:
		{
			START_TEST("test010", "test SIZE");

			/* start of the test sequence */
			TBBSizeEvent sw;
			
			sw.boardmask =itsboardmask; // boards tot test
			
			TESTC_ABORT(itsClient.send(sw), TBBTest::final);
		}
		break;

		case TBB_SIZE_ACK:
		{
			TBBSizeAckEvent ack(e);

			for (int boardnr = 0; boardnr < MAX_N_TBBOARDS;boardnr++) {
					TESTC_ABORT(ack.status_mask[boardnr] == TBB_SUCCESS, TBBTest::final);
				}
			LOG_INFO_STR("Size test OK");

			TRAN(TBBTest::final);
		}
		break;

		case F_DISCONNECTED:
		{
			port.close();

			FAIL_ABORT("unexpected disconnect", TBBTest::final);
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

GCFEvent::TResult TBBTest::final(GCFEvent& e, GCFPortInterface& /*port*/)
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

void TBBTest::run()
{
  start(); // make initial transition
  GCFTask::run();
}


int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("TBBDriver Test driver", &cerr);
  s.addTest(new TBBTest("TBBTest"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
