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
      // send write register message to RA test port
      RSPUpdstatusEvent updStatusEvent;
      struct timeval timeValNow;
      time(&timeValNow.tv_sec);
      timeValNow.tv_usec=0;
      updStatusEvent.timestamp.set(timeValNow);
      updStatusEvent.status=1;
      updStatusEvent.handle=1;
     
      EPA_Protocol::BoardStatus boardStatus;
      boardStatus.rsp.voltage_15 = 0;
      boardStatus.rsp.voltage_22 = 0;
      boardStatus.rsp.ffi = 0;
      boardStatus.bp.status = 0;
      boardStatus.bp.temp = static_cast<uint8>(27.13*100.0);
      boardStatus.ap[0].status = 0;
      boardStatus.ap[0].temp = static_cast<uint8>(27.23*100.0);
      boardStatus.ap[1].status = 0;
      boardStatus.ap[1].temp = static_cast<uint8>(27.33*100.0);
      boardStatus.ap[2].status = 0;
      boardStatus.ap[2].temp = static_cast<uint8>(27.43*100.0);
      boardStatus.ap[3].status = 0;
      boardStatus.ap[3].temp = static_cast<uint8>(27.53*100.0);
      boardStatus.eth.nof_frames = 0;
      boardStatus.eth.nof_errors = 0;
      boardStatus.eth.last_error = 0;
      boardStatus.eth.ffi0 = 0;
      boardStatus.eth.ffi1 = 0;
      boardStatus.eth.ffi2 = 0;
      boardStatus.read.seqnr = 0;
      boardStatus.read.error = 0;
      boardStatus.read.ffi = 0;
      boardStatus.write.seqnr = 0;
      boardStatus.write.error = 0;
      boardStatus.write.ffi = 0;
 
      EPA_Protocol::RCUStatus rcuStatus;
      std::bitset<8> rcuBitStatus;
      rcuBitStatus[7] = 1; // overflow
      rcuStatus.status = rcuBitStatus.to_ulong();
      updStatusEvent.sysstatus.board().resize(1);
      updStatusEvent.sysstatus.board()(0) = boardStatus;
      updStatusEvent.sysstatus.rcu().resize(1);
      updStatusEvent.sysstatus.rcu()(0) = rcuStatus;
 
/////////////////////////////////  Debug requiredSize berekening 
 
// volledig uitgeschreven gaat het goed:
      unsigned int sizeOFsignal = sizeof(updStatusEvent.signal);
      unsigned int sizeOFlength = sizeof(updStatusEvent.length);
      unsigned int sizeOFtimestamp = updStatusEvent.timestamp.getSize();
      unsigned int sizeOFstatus = sizeof(updStatusEvent.status);
      unsigned int sizeOFhandle = sizeof(updStatusEvent.handle);
      unsigned int sizeOFdimensions = updStatusEvent.sysstatus.board().dimensions()*sizeof(int32);
      unsigned int sizeOFarray = updStatusEvent.sysstatus.board().size()*sizeof(EPA_Protocol::BoardStatus);
      unsigned int sizeOFboard = sizeOFdimensions + sizeOFarray;
      sizeOFdimensions = updStatusEvent.sysstatus.rcu().dimensions()*sizeof(int32);
      sizeOFarray = updStatusEvent.sysstatus.rcu().size()*sizeof(EPA_Protocol::RCUStatus);
      unsigned int sizeOFrcu = sizeOFdimensions + sizeOFarray;
     
      unsigned int sizeOFsysstatus = sizeOFboard + sizeOFrcu;
 
      unsigned int requiredSize = sizeOFsignal + sizeOFlength
        + sizeOFtimestamp
       
        + sizeOFstatus
       
        + sizeOFhandle
       
        + sizeOFsysstatus
        ;
      LOG_INFO(formatString("sizeOf UPDSTATUS: %d",requiredSize));
 
// precies zoals in pack() gaat het niet goed:
      requiredSize = sizeof(updStatusEvent.signal) + sizeof(updStatusEvent.length)
	+ updStatusEvent.timestamp.getSize()
   
	+ sizeof(updStatusEvent.status)
   
	+ sizeof(updStatusEvent.handle)
   
	+ updStatusEvent.sysstatus.getSize()
	;
      LOG_INFO(formatString("sizeOf UPDSTATUS: %d",requiredSize));
 
// precies zoals in pack(), maar dan met uitgeschreven macro's gaat het weer wel goed:
      requiredSize = sizeof(updStatusEvent.signal) + sizeof(updStatusEvent.length)
	+ updStatusEvent.timestamp.getSize()
   
	+ sizeof(updStatusEvent.status)
   
	+ sizeof(updStatusEvent.handle)
   
//    + updStatusEvent.sysstatus.getSize()
	+ ((updStatusEvent.sysstatus.board().dimensions()*sizeof(int32)) + (updStatusEvent.sysstatus.board().size() * sizeof(EPA_Protocol::BoardStatus)))
	+ ((updStatusEvent.sysstatus.rcu().dimensions()*sizeof(int32)) + (updStatusEvent.sysstatus.rcu().size() * sizeof(EPA_Protocol::RCUStatus)))
	;
      LOG_INFO(formatString("sizeOf UPDSTATUS: %d",requiredSize));
 
      ////////////////////////////////////////////////  Debug requiredSize berekening
 
      //m_RSPserver.send(updStatusEvent);

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
      cout << "sw.time=" << sw.timestamp << endl;
      sw.rcumask.reset();
      sw.weights().resize(1, 1, N_BEAMLETS);

      sw.weights()(0, 0, Range::all()) = 0xbeaf;
	  
      sw.rcumask.set(0);

      TESTC_ABORT(m_server.send(sw), RSPTest::final);
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      cout << "ack.time=" << ack.timestamp << endl;

      TRAN(RSPTest::test002);
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
	  cout << "sw.time= " << sw.timestamp << endl;	  
	  sw.rcumask.reset();
	  sw.rcumask.set(0);
	  sw.cache = false;
	  
	  TESTC_ABORT(m_server.send(sw), RSPTest::final);
      }
      break;

      case RSP_GETWEIGHTSACK:
      {
	  RSPGetweightsackEvent ack(e);

	  cout << "ack.weights = " << ack.weights() << endl;
	  cout << "ack.time= " << ack.timestamp << endl;

	  TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);

	  TRAN(RSPTest::test003);
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
      cout << "sw.time=" << sw.timestamp << endl;
      sw.rcumask.reset();
      sw.weights().resize(1, 1, N_BEAMLETS);

      sw.weights()(0, 0, Range::all()) = 0xbeaf;
	  
      sw.rcumask.set(0);

      TESTC_ABORT(m_server.send(sw), RSPTest::final);
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      cout << "ack.time=" << ack.timestamp << endl;

      TRAN(RSPTest::test004);
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
      cout << "sw.time=" << sw.timestamp << endl;
      sw.rcumask.reset();

      // send weights for 10 timesteps
      sw.weights().resize(10, 1, N_BEAMLETS);

      sw.weights()(Range::all(), 0, Range::all()) = 0xbeaf;
	  
      sw.rcumask.set(0);

      TESTC_ABORT(m_server.send(sw), RSPTest::final);
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, RSPTest::final);
      cout << "ack.time=" << ack.timestamp << endl;

      TRAN(RSPTest::test005);
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

      ss.timestamp.setNow(10);
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
      cout << "ack.time=" << ack.timestamp << endl;

      TRAN(RSPTest::test006);
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
      cout << "ack.time=" << ack.timestamp << endl;

      LOG_INFO_STR("board=" << ack.sysstatus.board());
      LOG_INFO_STR("rcu="   << ack.sysstatus.rcu());
      
      TRAN(RSPTest::final);

      port.close();
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
