//#
//#  SweepTest.cc: implementation of SweepTest class
//# 
//#  This test creates N_BEAMLETS beams on the sky each beam
//#  only one subband wide. Each beam has a different direction
//#  ranging from -1 (horizon) through 0 (zenith) to 1 (other horizon).
//#  This can be used to visualize the beamshape for a single subband.
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
#include <APL/BS_Protocol/Pointing.h>

#include "SweepTest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <time.h>

using namespace LOFAR;
using namespace BS;
using namespace std;
using namespace EPA_Protocol;

SweepTest::SweepTest(string name, int subband)
    : GCFTask((State)&SweepTest::initial, name), Test("SweepTest"), m_subband(subband)
{
  registerProtocol(BS_PROTOCOL, BS_PROTOCOL_STRINGS);

  beam_server.init(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::SAP, BS_PROTOCOL);
}

SweepTest::~SweepTest()
{}

GCFEvent::TResult SweepTest::initial(GCFEvent& e, GCFPortInterface& port)
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
	  TRAN(SweepTest::enabled);
      }
      break;

      case F_DISCONNECTED:
      {
	  // only do 5 reconnects
	  if (disconnect_count++ > 5)
	  {
	      FAIL("timeout");
	      TRAN(SweepTest::done);
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

GCFEvent::TResult SweepTest::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int timerid = 0;

  static int beam_count = 0;
  static uint32 beam_handles[MEPHeader::N_BEAMLETS];
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	//
	// send beam allocation, select a single subband
	// this creates the first beam of N_BEAMLETS beams
	//
	BSBeamallocEvent alloc;
	alloc.name = "SweepTest";
	alloc.subarrayname = "FTS-1-LBA-RSP0";
	beam_count=0;
	alloc.allocation()[beam_count] = m_subband;

	TESTC(beam_server.send(alloc));
      }
      break;
      
    case BS_BEAMALLOCACK:
      {
	BSBeamallocackEvent ack(e);
	TESTC(BS_Protocol::SUCCESS == ack.status);
	if (BS_Protocol::SUCCESS != ack.status)
	  {
	    LOG_FATAL("Failed to allocate beam.");
	    exit(EXIT_FAILURE);
	  }

	beam_handles[beam_count] = ack.handle;
	LOG_DEBUG(formatString("got beam_handle=%d", ack.handle));

	//
	// keep allocating beams until we have N_BEAMLETS beams
	//
	if (++beam_count < MEPHeader::N_BEAMLETS)
	  {
	    BSBeamallocEvent alloc;
	    alloc.name = "SweepTest";
	    alloc.subarrayname = "FTS-1-LBA-RSP0";
	    alloc.allocation()[beam_count] = m_subband;
	  
	    TESTC(beam_server.send(alloc));
	  }
	else
	  {
	    // all beams created
	    
	    // send pointto commands from -90 through 0 to 90
	    BSBeampointtoEvent pointto;
	    
	    // this sends N_BEAMLETS pointto messages, one for each beam
	    for (int beam = 0; beam < MEPHeader::N_BEAMLETS; beam++)
	      {
		pointto.handle = beam_handles[beam];
		pointto.pointing = Pointing(::cos(((double)beam/MEPHeader::N_BEAMLETS)*M_PI),
					    0.0,
					    RTC::Timestamp::now(20),
					    Pointing::LOFAR_LMN);
		
		TESTC(beam_server.send(pointto));
	      }

	    // let the beamformer compute for 120 seconds
	    timerid = beam_server.setTimer((long)120);
	  }
      }
      break;

    case F_TIMER:
      {
	// stop task, beams will be freed automatically
	TRAN(SweepTest::done);
      }
      break;

    case F_DISCONNECTED:
      {
        FAIL("disconnected");
	port.close();
	TRAN(SweepTest::done);
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

GCFEvent::TResult SweepTest::done(GCFEvent& e, GCFPortInterface& /*port*/)
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

void SweepTest::run()
{
  start(); // make initial transition
  GCFScheduler::instance()->run();
}

int main(int argc, char** argv)
{
  LOG_INFO(formatString("Program %s has started", argv[0]));
  GCFScheduler::instance()->init(argc, argv);

  cout << "Subband index to plot: ";
  char buf[32];
  int subband = atoi(fgets(buf, 32, stdin));
  if (subband < 0 || subband > MEPHeader::N_SUBBANDS)
  {
      LOG_FATAL(formatString("Invalid subband index, should >= 0 && < %d", MEPHeader::N_BEAMLETS));
      exit(EXIT_FAILURE);
  }

  Suite s("Beam Server Process Test Suite", &cerr);
  s.addTest(new SweepTest("SweepTest", subband));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
