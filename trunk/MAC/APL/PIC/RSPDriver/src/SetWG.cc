//#
//#  SetWG.cc: implementation of SetWG class
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

#include "SetWG.h"

#include <PSAccess.h>
#include <GCF/ParameterSet.h>

#include <Suite/suite.h>
#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>
#include <gnuplot_i.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace LOFAR;
using namespace blitz;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

#define SAMPLE_FREQUENCY 120.0e6

#define START_TEST(_test_, _descr_) \
  setCurSubTest(#_test_, _descr_)

#define STOP_TEST() \
  reportSubTest()

#define FAIL_ABORT(_txt_, _final_state_) \
do { \
    FAIL(_txt_);  \
    TRAN(_final_state_); \
} while (0)

#define TESTC_ABORT(cond, _final_state_) \
do { \
  if (!TESTC(cond)) \
  { \
    TRAN(_final_state_); \
    break; \
  } \
} while (0)

#define TESTC_DESCR_ABORT(cond, _descr_, _final_state_) \
do { \
  if (!TESTC_DESCR(cond, _descr_)) \
  { \
    TRAN(_final_state_); \
    break; \
  } \
} while(0)

SetWG::SetWG(string name, int rcu, uint8 phase, uint8 ampl, double freq)
  : GCFTask((State)&SetWG::initial, name), Test(name),
    m_rcu(rcu), m_phase(phase), m_ampl(ampl), m_freq(freq)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

SetWG::~SetWG()
{}

GCFEvent::TResult SetWG::initial(GCFEvent& e, GCFPortInterface& port)
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
      if (!m_server.isConnected()) m_server.open();
    }
    break;

    case F_CONNECTED:
    {
      TRAN(SetWG::enabled);
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

GCFEvent::TResult SetWG::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("enabled", "SetWG");

      // subscribe to status updates
      RSPSetwgEvent wg;
 
      wg.timestamp.setNow();
      wg.rcumask.reset();
      if (m_rcu >= 0)
      {
	wg.rcumask.set(m_rcu);
      }
      else
      {
	for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL; i++)
	  wg.rcumask.set(i);
      }
	
      wg.settings().resize(1);

      if (m_freq > 1e-6 && m_freq < SAMPLE_FREQUENCY / 2.0) // within range?
      {
	wg.settings()(0).freq = (uint16)(((m_freq * (1 << 16)) / SAMPLE_FREQUENCY) + 0.5);
	wg.settings()(0).phase = m_phase;
	wg.settings()(0).ampl = m_ampl;
	wg.settings()(0).nof_samples = 512;
	wg.settings()(0).mode = WGSettings::MODE_CALC;
	wg.settings()(0)._pad = 0; // keep valgrind happy
      }
      else
      {
	wg.settings()(0).freq = 0;
	wg.settings()(0).phase = 0;
	wg.settings()(0).ampl = 0;
	wg.settings()(0).nof_samples = 512;
	wg.settings()(0).mode = WGSettings::MODE_OFF;
	wg.settings()(0)._pad = 0; // keep valgrind happy
      }
      
      if (!m_server.send(wg))
      {
	LOG_FATAL("failed to send RSPSetwgEvent");
	exit(EXIT_FAILURE);
      }
    }
    break;

    case RSP_SETWGACK:
    {
      RSPSetwgackEvent ack(e);

      LOG_INFO_STR("ack.time=" << ack.timestamp);

      if (SUCCESS != ack.status)
      {
	LOG_FATAL("negative ack");
	exit(EXIT_FAILURE);
      }
      else
      {
	// wait 5 seconds for the command to activate
	// before disconnecting
	m_server.setTimer(5.0);
      }
    }
    break;

    case F_TIMER:
    {
      // we're done
      TRAN(SetWG::final);
    }
    break;
	
    case F_DISCONNECTED:
    {
      port.close();
      TRAN(SetWG::initial);
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

GCFEvent::TResult SetWG::final(GCFEvent& e, GCFPortInterface& /*port*/)
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

void SetWG::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  char buf[32];

  //
  // Read parameters
  //
  try
  {
    GCF::ParameterSet::instance()->adoptFile("RSPDriverPorts.conf");
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  //
  // Read amplitude
  //
  cout << "Amplitude % (0-100):";
  double ampl = atof(fgets(buf, 32, stdin));
  if (ampl < 0 || ampl > 100.0)
  {
    LOG_FATAL(formatString("Invalid amplitude specification should be between 0 and 100"));
    exit(EXIT_FAILURE);
  }

  //
  // Read frequency
  //
  cout << "Frequency (in MHz, [0:" << SAMPLE_FREQUENCY / 2.0 << "]):";
  double freq = atof(fgets(buf, 32, stdin));
  if (freq < 0 || freq > SAMPLE_FREQUENCY / 2.0)
  {
    LOG_FATAL(formatString("Invalid frequency, should be in range [0:%d]", SAMPLE_FREQUENCY / 2.0));
    exit(EXIT_FAILURE);
  }

  //
  // Read RCU
  //
  cout << "Which RCU? (-1 means all): ";
  int rcu = atoi(fgets(buf, 32, stdin));
  if (rcu < -1 || rcu >= GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL)
  {
    LOG_FATAL(formatString("Invalid RCU index, should be >= -1 && < %d; -1 indicates all RCU's",
			   GET_CONFIG("RS.N_RSPBOARDS", i) * GET_CONFIG("RS.N_BLPS", i) * MEPHeader::N_POL));
    exit(EXIT_FAILURE);
  }
  
  Suite s("SetWG", &cerr);
  s.addTest(new SetWG("SetWG", rcu, (uint8)0, (uint8)(ampl*(double)(1<<7)/100.0), freq)); // set phase to 0 for now
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
