//#
//#  ABSSweepTest.cc: implementation of SweepTest class
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

// this include needs to be first!

#include <Suite/suite.h>
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"
#include "ABSDirection.h"

#include "ABSSweepTest.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <time.h>

#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace ABS;
using namespace std;
using namespace LOFAR;

SweepTest::SweepTest(string name, int subband)
    : GCFTask((State)&SweepTest::initial, name), Test("SweepTest"), m_subband(subband)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  beam_server.init(*this, "beam_server", GCFPortInterface::SAP, ABS_PROTOCOL);
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
  static int beam_handle = -1;

  static int beam_count = 0;
  
  switch (e.signal)
  {
      case F_ENTRY:
      {
	  //
	  // send beam allocation, select a single subband
	  // this creates the first beam of N_BEAMLETS beams
	  //
	  ABSBeamallocEvent alloc;
	  alloc.spectral_window = 0;
	  alloc.n_subbands = 1;
	  memset(alloc.subbands, 0, sizeof(alloc.subbands));
	  alloc.subbands[0] = m_subband;

	  TESTC(beam_server.send(alloc));
      }
      break;
      
      case ABS_BEAMALLOC_ACK:
      {
	ABSBeamallocAckEvent ack(e);
	TESTC(ABS_Protocol::SUCCESS == ack.status);
	if (ABS_Protocol::SUCCESS != ack.status)
	{
	  LOG_FATAL("Failed to allocate beam.");
	  exit(EXIT_FAILURE);
	}

	beam_handle = ack.handle;
	LOG_DEBUG(formatString("got beam_handle=%d", beam_handle));

	//
	// keep allocating beams until we have N_BEAMLETS beams
	//
	if (++beam_count < MEPHeader::N_BEAMLETS)
	{
	  ABSBeamallocEvent alloc;
	  alloc.spectral_window = 0;
	  alloc.n_subbands = 1;
	  memset(alloc.subbands, 0, sizeof(alloc.subbands));
	  alloc.subbands[0] = m_subband;
	  
	  TESTC(beam_server.send(alloc));
	}
	else
	{
	    // all beams created
	    
	    // send pointto commands from -90 through 0 to 90
	    ABSBeampointtoEvent pointto;
	    pointto.type=(int)Direction::LOFAR_LMN;
	    
	    time_t now = time(0);

	    // this sends N_BEAMLETS pointto messages, one for each beam
	    for (int beam = 0; beam < MEPHeader::N_BEAMLETS; beam++)
	    {
		pointto.handle = beam;
		pointto.time = now + 20;
		pointto.angle[0]=0.0;
		pointto.angle[1]=cos(((double)beam/MEPHeader::N_BEAMLETS)*M_PI);
		
		TESTC(beam_server.send(pointto));
	    }

	    // let the beamformer compute for 120 seconds
	    timerid = beam_server.setTimer((long)120);
	}
      }
      break;

      case F_TIMER:
      {
	  // done => send BEAMFREE
	  ABSBeamfreeEvent beamfree;
	  beamfree.handle = beam_handle;

	  TESTC(beam_server.send(beamfree));
      }
      break;

      case ABS_BEAMFREE_ACK:
      {
	ABSBeamfreeAckEvent ack(e);
	TESTC(ABS_Protocol::SUCCESS == ack.status);
	TESTC(beam_handle == ack.handle);

	// test completed, next test
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
    GCFTask::stop();
    break;
  }

  return status;
}

void SweepTest::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
#if 0
  char prop_path[PATH_MAX];
  const char* mac_config = getenv("MAC_CONFIG");

  snprintf(prop_path, PATH_MAX-1,
	   "%s/%s", (mac_config?mac_config:"."),
	   "log4cplus.properties");
  INIT_LOGGER(prop_path);
#endif
  LOG_INFO(formatString("Program %s has started", argv[0]));
  GCFTask::init(argc, argv);

  cout << "Subband index to plot: ";
  char buf[32];
  int subband = atoi(fgets(buf, 32, stdin));
  if (subband < 0 || subband > MEPHeader::N_BEAMLETS)
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
