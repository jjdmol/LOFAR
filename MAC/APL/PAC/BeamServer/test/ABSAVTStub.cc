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

// this include needs to be first!

#include "suite.h"
#define DECLARE_SIGNAL_NAMES
#include "ABS_Protocol.ph"

#include "ABSAVTStub.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace ABS;
using namespace std;
using namespace LOFAR;

AVTStub::AVTStub(string name)
  : GCFTask((State)&AVTStub::initial, name), Test("AVTStub")
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  beam_server.init(*this, "beam_server", GCFPortInterface::SAP, ABS_PROTOCOL);
}

AVTStub::~AVTStub()
{}

GCFEvent::TResult AVTStub::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  LOG_DEBUG(formatString("initial received event on port %s", port.getName().c_str()));

  switch(e.signal)
  {
      case F_INIT_SIG:
      {
      }
      break;

      case F_ENTRY_SIG:
      {
	  beam_server.open();
      }
      break;

      case F_CONNECTED_SIG:
      {
	  LOG_INFO(formatString("port %s connected", port.getName().c_str()));
	  TRAN(AVTStub::test001);
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  port.setTimer((long)2);
      }
      break;

      case F_TIMER_SIG:
      {
	  // try again
	  beam_server.open();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test001(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  int timerid = 0;

  switch (e.signal)
  {
      case F_ENTRY_SIG:
      {
	LOG_INFO("running test001");
  
	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// start test by sending beam alloc
	ABSBeamallocEvent alloc;
	alloc.beam_index = 0;
	alloc.spectral_window = 0;
	alloc.n_subbands = 1;
	memset(alloc.subbands, 0, sizeof(alloc.subbands));

	_test(sizeof(alloc) == beam_server.send(alloc));
      }
      break;

      case ABS_BEAMALLOC_ACK:
      {
	ABSBeamalloc_AckEvent* ack = static_cast<ABSBeamalloc_AckEvent*>(&e);
	_test(SUCCESS == ack->status);
	_test(0 == ack->beam_index);

	// beam allocated, now free it
	ABSBeamfreeEvent beamfree;
	beamfree.beam_index = 0;

	_test(sizeof(beamfree) == beam_server.send(beamfree));
      }
      break;

      case ABS_BEAMFREE_ACK:
      {
	ABSBeamfree_AckEvent* ack = static_cast<ABSBeamfree_AckEvent*>(&e);
	_test(SUCCESS == ack->status);
	_test(0 == ack->beam_index);

	// test completed, next test
	TRAN(AVTStub::test002);
      }
      break;

      case F_TIMER_SIG:
      {
	// abort test
	beam_server.close();
	_fail("timeout");
	TRAN(AVTStub::done);
      }
      break;

      case F_EXIT_SIG:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test002(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  int timerid = 0;
  
  switch (e.signal)
  {
      case F_ENTRY_SIG:
      {
	LOG_INFO("running test002");

	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// start test by sending beam alloc
	ABSBeamallocEvent alloc;
	alloc.beam_index = 0;
	alloc.spectral_window = 0;
	alloc.n_subbands = 1;
	memset(alloc.subbands, 0, sizeof(alloc.subbands));

	_test(sizeof(alloc) == beam_server.send(alloc));
      }
      break;

      case ABS_BEAMALLOC_ACK:
      {
	ABSBeamalloc_AckEvent* ack = static_cast<ABSBeamalloc_AckEvent*>(&e);
	_test(SUCCESS == ack->status);
	_test(0 == ack->beam_index);

	// send pointto command
	ABSBeampointtoEvent pointto;
	pointto.beam_index = 0;
	gettimeofday(&pointto.time,0);
	pointto.type=3;
	pointto.angle1=0.0;
	pointto.angle2=-1.0;

	_test(sizeof(pointto) == beam_server.send(pointto));

	// beam pointed, now free it
	ABSBeamfreeEvent beamfree;
	beamfree.beam_index = 0;

	_test(sizeof(beamfree) == beam_server.send(beamfree));
      }
      break;

      case ABS_BEAMFREE_ACK:
      {
	ABSBeamfree_AckEvent* ack = static_cast<ABSBeamfree_AckEvent*>(&e);
	_test(SUCCESS == ack->status);
	_test(0 == ack->beam_index);

	// test completed, next test
	TRAN(AVTStub::test003);
      }
      break;

      case F_TIMER_SIG:
      {
	// abort test
	beam_server.close();
	_fail("timeout");
	TRAN(AVTStub::done);
      }
      break;

      case F_EXIT_SIG:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test003(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  int timerid = 0;
  
  switch (e.signal)
  {
      case F_ENTRY_SIG:
      {
	LOG_INFO("running test003");

	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// send wgenable
	ABSWgenableEvent wgenable;
	wgenable.frequency=1e6;
	wgenable.amplitude=128;
	wgenable.sample_period=2;

	_test(sizeof(wgenable) == beam_server.send(wgenable));

	// start test by sending beam alloc
	ABSBeamallocEvent alloc;
	alloc.beam_index = 0;
	alloc.spectral_window = 0;
	alloc.n_subbands = 1;
	memset(alloc.subbands, 0, sizeof(alloc.subbands));

	_test(sizeof(alloc) == beam_server.send(alloc));
      }
      break;

      case ABS_BEAMALLOC_ACK:
      {
	ABSBeamalloc_AckEvent* ack = static_cast<ABSBeamalloc_AckEvent*>(&e);
	_test(SUCCESS == ack->status);
	_test(0 == ack->beam_index);

	// send pointto command
	ABSBeampointtoEvent pointto;
	pointto.beam_index = 0;
	gettimeofday(&pointto.time,0);
	pointto.type=3;
	pointto.angle1=0.0;
	pointto.angle2=-1.0;

	_test(sizeof(pointto) == beam_server.send(pointto));

	// beam pointed, now free it
	ABSBeamfreeEvent beamfree;
	beamfree.beam_index = 0;

	_test(sizeof(beamfree) == beam_server.send(beamfree));
      }
      break;

      case ABS_BEAMFREE_ACK:
      {
	ABSBeamfree_AckEvent* ack = static_cast<ABSBeamfree_AckEvent*>(&e);
	_test(SUCCESS == ack->status);
	_test(0 == ack->beam_index);

	// send wgdisable
	_test(sizeof(GCFEvent) == beam_server.send(GCFEvent(ABS_WGDISABLE)));

	// test completed, next test
	TRAN(AVTStub::test004);
      }
      break;

      case F_TIMER_SIG:
      {
	// abort test
	beam_server.close();
	_fail("timeout");
	TRAN(AVTStub::done);
      }
      break;

      case F_EXIT_SIG:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult AVTStub::test004(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  int timerid = 0;
  static int loop = 0;
  
  switch (e.signal)
  {
      case F_ENTRY_SIG:
      {
	LOG_INFO("running test004");

	// start test timer, test should finish
	// within 2 seconds
	timerid = beam_server.setTimer((long)2);

	// invalid n_subbands in beam alloc
	ABSBeamallocEvent alloc;
	alloc.beam_index = 0;
	alloc.spectral_window = 0;
	alloc.n_subbands = N_BEAMLETS+1;
	memset(alloc.subbands, 0, sizeof(alloc.subbands));

	_test(sizeof(alloc) == beam_server.send(alloc));
      }
      break;

      case ABS_BEAMALLOC_ACK:
      {
	ABSBeamalloc_AckEvent* ack = static_cast<ABSBeamalloc_AckEvent*>(&e);
	_test(ERR_RANGE == ack->status);

	if (loop++ == 0)
	{
	  // send invalid beam_index
	  ABSBeamallocEvent alloc;
	  alloc.beam_index = -1;
	  alloc.spectral_window = 0;
	  alloc.n_subbands = 1;
	  memset(alloc.subbands, 0, sizeof(alloc.subbands));

	  _test(sizeof(alloc) == beam_server.send(alloc));
	}
	else if (loop++ == 1)
	{
	  // send invalid spectral window index
	  ABSBeamallocEvent alloc;
	  alloc.beam_index = 0;
	  alloc.spectral_window = -1;
	  alloc.n_subbands = 1;
	  memset(alloc.subbands, 0, sizeof(alloc.subbands));

	  _test(sizeof(alloc) == beam_server.send(alloc));
	}
	else if (loop++ == 2)
	{
	  // send large beam index
	  ABSBeamallocEvent alloc;
	  alloc.beam_index = N_BEAMLETS+1;
	  alloc.spectral_window = 0;
	  alloc.n_subbands = 1;
	  memset(alloc.subbands, 0, sizeof(alloc.subbands));

	  _test(sizeof(alloc) == beam_server.send(alloc));
	}
	else if (loop++ == 3)
	{
	  // send invalid n_subbands (-1)
	  ABSBeamallocEvent alloc;
	  alloc.beam_index = 0;
	  alloc.spectral_window = 0;
	  alloc.n_subbands = -1;
	  memset(alloc.subbands, 0, sizeof(alloc.subbands));

	  _test(sizeof(alloc) == beam_server.send(alloc));
	}
	else TRAN(AVTStub::done);
      }
      break;

      case F_TIMER_SIG:
      {
	// abort test
	beam_server.close();
	_fail("timeout");
	TRAN(AVTStub::done);
      }
      break;

      case F_EXIT_SIG:
      {
	// before leaving, cancel the timer
	beam_server.cancelTimer(timerid);
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }

  return status;
}

GCFEvent::TResult AVTStub::done(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
  case F_ENTRY_SIG:
    GCFTask::stop();
    break;
  }

  return status;
}

void AVTStub::run()
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

  Suite s("Beam Server Process Test Suite", &cerr);
  s.addTest(new AVTStub("AVTStub"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
