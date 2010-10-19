//#
//#  ABSSetFreq.cc: implementation of SetFreq class
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
#include "ABSDirection.h"

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

#include "ABS_Protocol.ph"
#include "test.h"

#include <GCF/GCF_Control.h>
#include <GCF/GCF_ETHRawPort.h>

using namespace ABS;
using namespace std;
using namespace LOFAR;

namespace ABS
{
    class SetFreq : public GCFTask, public Test
    {
    public:
	/**
	 * The constructor of the SetFreq task.
	 * @param name The name of the task. The name is used for looking
	 * up connection establishment information using the GTMNameService and
	 * GTMTopologyService classes.
	 */
	SetFreq(string name, double freq);
	virtual ~SetFreq();

	// state methods

	/**
	 * The initial state. In this state the beam_server port
	 * is opened.
	 */
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);
	GCFEvent::TResult setfreq(GCFEvent& e, GCFPortInterface &p);	
	GCFEvent::TResult done   (GCFEvent& e, GCFPortInterface &p);

	/**
	 * The test run method. This should start the task
	 */
	void run();

    private:
	// member variables

    private:
	// ports
	GCFPort       beam_server;
	double m_freq;
    };
};

SetFreq::SetFreq(string name, double freq)
    : GCFTask((State)&SetFreq::initial, name), Test("SetFreq"), m_freq(freq)
{
  registerProtocol(ABS_PROTOCOL, ABS_PROTOCOL_signalnames);

  beam_server.init(*this, "beam_server", GCFPortInterface::SAP, ABS_PROTOCOL);
}

SetFreq::~SetFreq()
{}

GCFEvent::TResult SetFreq::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  static int disconnect_count = 0;

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
	  LOG_DEBUG(formatString("port %s connected", port.getName().c_str()));

	  TRAN(SetFreq::setfreq);
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  // only do 5 reconnects
	  if (disconnect_count++ > 5)
	  {
	      _fail("timeout");
	      TRAN(SetFreq::done);
	  }
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
      {
	  status = GCFEvent::NOT_HANDLED;
      }
      break;
  }

  return status;
}

GCFEvent::TResult SetFreq::setfreq(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  LOG_DEBUG(formatString("initial received event on port %s", port.getName().c_str()));

  switch(e.signal)
  {
      case F_ENTRY_SIG:
      {
	  // this should finish within 1 second
	  beam_server.setTimer((long)1);

	  // send wgenable
	  ABSWgsettingsEvent wgs;
	  wgs.frequency=m_freq;
	  wgs.amplitude=128; // was 128
	  wgs.sample_period=2;

	  _test(sizeof(wgs) == beam_server.send(wgs));
      }
      break;

      case ABS_WGSETTINGS_ACK:
      {
	  // check acknowledgement
	  ABSWgsettings_AckEvent* wgsa = static_cast<ABSWgsettings_AckEvent*>(&e);
	  _test(SUCCESS == wgsa->status);
	  
	  // send WGENABLE
	  _test(sizeof(GCFEvent) == beam_server.send(GCFEvent(ABS_WGENABLE)));
	  
	  TRAN(SetFreq::done);
      }
      break;

      case F_DISCONNECTED_SIG:
      {
	  _fail("disconnected");
	  port.close();
	  TRAN(SetFreq::done);
      }
      break;

      case F_TIMER_SIG:
      {
	  // too late
	  _fail("timeout");
	  TRAN(SetFreq::done);
      }
      break;

      case F_EXIT_SIG:
      {
	  beam_server.cancelAllTimers();
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

GCFEvent::TResult SetFreq::done(GCFEvent& e, GCFPortInterface& /*port*/)
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

void SetFreq::run()
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

  cout << "Frequency to select: ";
  char buf[32];
  double freq = atof(fgets(buf, 32, stdin));
  if (freq < 0.0 | freq > 20e6)
  {
      LOG_FATAL(formatString("Invalid frequency, should >= 0 && < %f", 20e6));
      exit(EXIT_FAILURE);
  }

  Suite s("Beam Server Process Test Suite", &cerr);
  s.addTest(new SetFreq("SetFreq", freq));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
