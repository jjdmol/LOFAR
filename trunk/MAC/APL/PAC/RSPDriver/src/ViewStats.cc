//#
//#  ViewStats.cc: implementation of ViewStats class
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

#include "ViewStats.h"

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

ViewStats::ViewStats(string name, int type, int rcu)
  : GCFTask((State)&ViewStats::initial, name), Test(name),
    m_type(type), m_rcu(rcu)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

ViewStats::~ViewStats()
{}

GCFEvent::TResult ViewStats::initial(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(ViewStats::enabled);
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

GCFEvent::TResult ViewStats::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      START_TEST("enabled", "test UPDSTATS");

      // subscribe to status updates
      RSPSubstatsEvent substats;

      substats.timestamp.setNow();
      substats.rcumask.reset();
      substats.rcumask.set(m_rcu);
      substats.period = 1;
      substats.type = m_type;
      substats.reduction = SUM;
      
      TESTC_ABORT(m_server.send(substats) > 0, ViewStats::final);
    }
    break;

    case RSP_SUBSTATSACK:
    {
      RSPSubstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, ViewStats::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);
    }
    break;

    case RSP_UPDSTATS:
    {
      RSPUpdstatsEvent upd(e);

      TESTC_ABORT(upd.status == SUCCESS, ViewStats::final);
      LOG_INFO_STR("upd.time=" << upd.timestamp);
      LOG_INFO_STR("upd.handle=" << upd.handle);
      
      LOG_INFO_STR("upd.stats=" << upd.stats());

      plot_statistics(upd.stats());
    }
    break;
    
    case RSP_UNSUBSTATSACK:
    {
      RSPUnsubstatsackEvent ack(e);

      TESTC_ABORT(ack.status == SUCCESS, ViewStats::final);
      LOG_INFO_STR("ack.time=" << ack.timestamp);

      LOG_INFO_STR("ack.handle=" << ack.handle);

      port.close();
      TRAN(ViewStats::final);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();

      FAIL_ABORT("unexpected disconnect", ViewStats::final);
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

GCFEvent::TResult ViewStats::final(GCFEvent& e, GCFPortInterface& /*port*/)
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

void ViewStats::plot_statistics(Array<complex<double>, 3>& stats)
{
  static gnuplot_ctrl* handle = 0;
  int n_freqbands = stats.extent(3);

  Array<double, 1> freq(n_freqbands);
  Array<double, 1> value(n_freqbands);

  // initialize the freq array
  firstIndex i;
  
  if (!handle)
  {
    handle = gnuplot_init();
    if (!handle) return;

    gnuplot_setstyle(handle,   "steps");
    gnuplot_cmd(handle, "set grid x y");
  }

  if (Statistics::SUBBAND_POWER == m_type
      || Statistics::BEAMLET_POWER == m_type)
  {
    // add real and imaginary part to get power
    value  = real(stats(0, 0, Range::all()));
    value += imag(stats(0, 0, Range::all()));

    // signal + 1e-6 and +10dB calibrated this to the Marconi signal generator
    value = log10((value + 1e-6) / (1.0*(1<<16))) * 10.0 + 10.0;

    // set yrange for power
    gnuplot_cmd(handle, "set ytics -100,10");
    gnuplot_cmd(handle, "set yrange [-100:20]");
  }
  else // MEAN
  {
    value =  real(stats(0, 0, Range::all())) * real(stats(0, 0, Range::all()));
    value += imag(stats(0, 0, Range::all())) * imag(stats(0, 0, Range::all()));
    value /= (1<<16);
    value /= (1<<16);
    value -= 1;

    gnuplot_set_xlabel(handle, "beamlet");
    gnuplot_set_ylabel(handle, "power");

    // set yrange for mean
    gnuplot_cmd(handle, "set ytics -1.25,.1");
    gnuplot_cmd(handle, "set yrange [-1.25:1.25]");
  }

  if (m_type < Statistics::BEAMLET_MEAN)
  {
    freq = i * (20.0 / n_freqbands); // calculate frequency in MHz
    gnuplot_set_xlabel(handle, "Frequency (MHz)");
  }
  else
  {
    freq = i; // selected beamlet index
    gnuplot_set_xlabel(handle, "Beamlet index");
  }

  gnuplot_resetplot(handle);

  char* title = 0;
  switch (m_type)
  {
    case Statistics::SUBBAND_MEAN:
      title = "Subband Mean Value";
      break;
    case Statistics::SUBBAND_POWER:
      title = "Subband Power";
      break;
    case Statistics::BEAMLET_MEAN:
      title = "Beamlet Mean Value";
      break;
    case Statistics::BEAMLET_POWER:
      title = "Beamlet Power";
      break;
    default:
      title = "ERROR: Invalid m_type";
      break;
  }

  gnuplot_plot_xy(handle, freq.data(), value.data(), n_freqbands, title);
}

void ViewStats::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  char buf[32];

  cout << "Type of stat [0==SUBBAND_MEAN, 1==SUBBAND_POWER, 2=BEAMLET_MEAN, 3=BEAMLET_POWER]:";
  int type = atoi(fgets(buf, 32, stdin));
  if (type < 0 || type >= Statistics::N_STAT_TYPES)
  {
    LOG_FATAL(formatString("Invalid type of stat, should be >= 0 && < %d", Statistics::N_STAT_TYPES));
    exit(EXIT_FAILURE);
  }

  cout << "Which RCU? ";
  int rcu = atoi(fgets(buf, 32, stdin));  
  if (rcu < 0 || rcu > 3 * 2)
  {
    LOG_FATAL(formatString("Invalid RCU index, should be >= 0 && < %d", N_BLP * 2));
    exit(EXIT_FAILURE);
  }
  
  Suite s("ViewStats", &cerr);
  s.addTest(new ViewStats("ViewStats", type, rcu));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
