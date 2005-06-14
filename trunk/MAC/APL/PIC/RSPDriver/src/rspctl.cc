//#
//#  rspctl.cc: command line interface to the RSPDriver
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
//#

//#
//# Usage:
//#
//# rspctl --weights        [--select=<set>] # get weights
//# rspctl --weights=(0|1)  [--select=<set>] # set weights
//# rspctl --subbands       [--select=<set>] # get subband selection
//# rspctl --subbands=<set> [--select=<set>] # set subband selection
//# rspctl --rcu            [--select=<set>] # get RCU control
//# rspctl --rcu=0xce       [--select=<set>] # set RCU control
//# rspctl --wg             [--select=<set>] # get waveform generator settings
//# rspctl --wg=freq        [--select=<set>] # set wg freq is in Hz (float)
//# rspctl --status         [--select=<set>] # get status
//# rspctl --statistics[=(subband|beamlet)]
//#                         [--select=<set>] # get subband (default) or beamlet statistics
//# rspctl --version        [--select=<set>] # get version information
//#

#include "rspctl.h"
#include "RSP_Protocol.ph"
#include "EPA_Protocol.ph"

#include <PSAccess.h>

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>
#include <getopt.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <set>

#include <gnuplot_i.h>

using namespace std;
using namespace LOFAR;
using namespace blitz;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace rspctl;
using namespace RTC;

// local funtions
static void usage();
static Command* parse_options(int argc, char** argv);

//
// Sample frequency, should be queried from the RSPDriver
//
#define SAMPLE_FREQUENCY 160.0e6

static std::set<int> strtoset(const char* str, int max)
{
  string inputstring(str);
  char* start = (char*)inputstring.c_str();
  char* end   = 0;
  bool  range = false;
  long prevval = 0;
  set<int> resultset;

  resultset.clear();

  while (start)
  {
    long val = strtol(start, &end, 10); // read decimal numbers
    start = (end ? (*end ? end + 1 : 0) : 0); // advance
    if (val >= max || val < 0)
    {
      cerr << "Error: value " << val << " out of range" << endl;
      resultset.clear();
      return resultset;
    }

    if (end)
    {
      switch (*end)
      {
	case ',':
	case 0:
	{
	  if (range)
	  {
	    if (0 == prevval && 0 == val) val = max - 1;
	    if (val < prevval)
	    {
	      cerr << "Error: invalid range specification" << endl;
	      resultset.clear();
	      return resultset;
	    }
	    for (long i = prevval; i <= val; i++) resultset.insert(i);
	  }
	    
	  else
	  {
	    resultset.insert(val);
	  }
	  range=false;
	}
	break;

	case ':':
	  range=true;
	  break;

	default:
	  cerr << "Error: invalid character " << *end << endl;
	  resultset.clear();
	  return resultset;
	  break;
      }
    }
    prevval = val;
  }

  return resultset;
}

WeightsCommand::WeightsCommand()
{
}

void WeightsCommand::send(GCFPortInterface& port)
{
  if (getMode()) {
    // GET
    RSPGetweightsEvent getweights;

    getweights.timestamp = Timestamp(0,0);

    bitset<MAX_N_RCUS> mask = getRCUMask();
    for (int rcu = 0; rcu < get_nrcus(); rcu++) {
      if (mask[rcu]) getweights.blpmask.set(rcu/2);
    }

    getweights.cache = true;

    port.send(getweights);
  } else {
    // SET
    RSPSetweightsEvent setweights;
    setweights.timestamp = Timestamp(0,0);
    bitset<MAX_N_RCUS> mask = getRCUMask();
    for (int rcu = 0; rcu < get_nrcus(); rcu++) {
      if (mask[rcu]) {
	cerr << "blpmask[" << rcu/2 << "] set" << endl;
	setweights.blpmask.set(rcu/2);
      }
    }

    cerr << "blpmask.count()=" << setweights.blpmask.count() << endl;

    setweights.weights().resize(1, get_nrcus()/2, MEPHeader::N_BEAMLETS, MEPHeader::N_POL);

    // -1 < m_value <= 1
    double value = m_value;
    value *= (1<<15)-1;
    setweights.weights() = complex<int16>((int16)value,0);

    port.send(setweights);
  }
}

GCFEvent::TResult WeightsCommand::ack(GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case RSP_GETWEIGHTSACK:
      {
	RSPGetweightsackEvent ack(e);
	bitset<MAX_N_RCUS> mask = getRCUMask();

	if (SUCCESS == ack.status) {
	  int blpin = 0;
	  for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {

	    if (mask[rcuout]) {
	      printf("RCU[%02d].weights=", rcuout);
	      cout << ack.weights()(0, blpin++, Range::all(), Range::all()) << endl;
	      rcuout++;
	    }
	  }
	} else {
	  cerr << "Error: RSP_GETWEIGHTS command failed." << endl;
	}
      }
      break;

    case RSP_SETWEIGHTSACK:
      {
	RSPSetweightsackEvent ack(e);

	if (SUCCESS != ack.status) {
	  cerr << "Error: RSP_SETWEIGHTS command failed." << endl;
	}
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  GCFTask::stop();

  return status;
}

SubbandsCommand::SubbandsCommand()
{
}

void SubbandsCommand::send(GCFPortInterface& port)
{
  if (getMode()) {
    // GET
    RSPGetsubbandsEvent getsubbands;

    getsubbands.timestamp = Timestamp(0,0);

    bitset<MAX_N_RCUS> mask = getRCUMask();
    for (int rcu = 0; rcu < get_nrcus(); rcu++) {
      if (mask[rcu]) getsubbands.blpmask.set(rcu/2);
    }

    getsubbands.cache = true;

    port.send(getsubbands);
  } else {
    // SET
    RSPSetsubbandsEvent setsubbands;
    setsubbands.timestamp = Timestamp(0,0);
    bitset<MAX_N_RCUS> mask = getRCUMask();
    for (int rcu = 0; rcu < get_nrcus(); rcu++) {
      if (mask[rcu]) {
	cerr << "blpmask[" << rcu/2 << "] set" << endl;
	setsubbands.blpmask.set(rcu/2);
      }
    }

    cerr << "blpmask.count()=" << setsubbands.blpmask.count() << endl;

    setsubbands.subbands().resize(1,m_subbandset.size()*2);

    std::set<int>::iterator it;
    int i;
    for (i = 0, it = m_subbandset.begin(); it != m_subbandset.end(); it++, i+=2) {
      setsubbands.subbands()(0, i)   = (*it)*2;
      setsubbands.subbands()(0, i+1) = (*it)*2+1;
    }

    port.send(setsubbands);
  }
}

GCFEvent::TResult SubbandsCommand::ack(GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case RSP_GETSUBBANDSACK:
      {
	RSPGetsubbandsackEvent ack(e);
	bitset<MAX_N_RCUS> mask = getRCUMask();

	if (SUCCESS == ack.status) {
	  int blpin = 0;
	  for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {

	    if (mask[rcuout]) {
	      printf("RCU[%02d].subbands=", rcuout);
	      cout << ack.subbands()(blpin++, Range::all()) << endl;
	      rcuout++;
	    }
	  }
	} else {
	  cerr << "Error: RSP_GETSUBBANDS command failed." << endl;
	}
      }
      break;

    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);

	if (SUCCESS != ack.status) {
	  cerr << "Error: RSP_SETSUBBANDS command failed." << endl;
	}
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  GCFTask::stop();

  return status;
}

RCUCommand::RCUCommand()
{
}

void RCUCommand::send(GCFPortInterface& port)
{
  if (getMode()) {
    // GET
    RSPGetrcuEvent getrcu;

    getrcu.timestamp = Timestamp(0,0);
    getrcu.rcumask = getRCUMask();
    getrcu.cache = true;

    port.send(getrcu);
  } else {
    // SET
    RSPSetrcuEvent setrcu;
    setrcu.timestamp = Timestamp(0,0);
    setrcu.rcumask = getRCUMask();
      
    setrcu.settings().resize(1);
    setrcu.settings()(0).value = m_control;

    port.send(setrcu);
  }
}

GCFEvent::TResult RCUCommand::ack(GCFEvent& e)
{
  switch (e.signal)
    {
    case RSP_GETRCUACK:
      {
	RSPGetrcuackEvent ack(e);
	bitset<MAX_N_RCUS> mask = getRCUMask();

	if (SUCCESS == ack.status) {
	  int rcuin = 0;
	  for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {

	    if (mask[rcuout]) {
	      printf("RCU[%02d].status=0x%02x\n",
		     rcuout, ack.settings()(rcuin++).value);
	    }
	  }
	} else {
	  cerr << "Error: RSP_GETRCU command failed." << endl;
	}
      }
      break;

    case RSP_SETRCUACK:
      {
	RSPSetrcuackEvent ack(e);

	if (SUCCESS != ack.status) {
	  cerr << "Error: RSP_SETRCU command failed." << endl;
	}
      }
    }

  GCFTask::stop();

  return GCFEvent::HANDLED;
}

WGCommand::WGCommand() :
  m_frequency(0.0),
  m_phase(0),
  m_amplitude(100)
{
}

void WGCommand::send(GCFPortInterface& port)
{
  if (getMode()) {
    // GET
    RSPGetwgEvent wgget;
    wgget.timestamp = Timestamp(0,0);
    wgget.rcumask = getRCUMask();
    wgget.cache = true;
    port.send(wgget);

  } else {
    // SET
    RSPSetwgEvent wgset;

    wgset.timestamp = Timestamp(0,0);
    wgset.rcumask = getRCUMask();
    wgset.settings().resize(1);
    wgset.settings()(0).freq = (uint16)(((m_frequency * (1<<16)) / SAMPLE_FREQUENCY) + 0.5);
    wgset.settings()(0).phase = m_phase;
    wgset.settings()(0).ampl = m_amplitude;
    wgset.settings()(0).nof_samples = N_WAVE_SAMPLES;

    if (m_frequency > 1e-6) {
      wgset.settings()(0).mode = WGSettings::MODE_CALC;
    } else {
      wgset.settings()(0).mode = WGSettings::MODE_OFF;
    }
    wgset.settings()(0).preset = 0; // or one of PRESET_[SINE|SQUARE|TRIANGLE|RAMP]

    port.send(wgset);
  }
}

GCFEvent::TResult WGCommand::ack(GCFEvent& e)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case RSP_GETWGACK:
      {
	RSPGetwgackEvent ack(e);
      
	if (SUCCESS == ack.status) {
	  
	  // print settings
	  bitset<MAX_N_RCUS> mask = getRCUMask();
	  int rcuin = 0;
	  for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {
	    
	    if (mask[rcuout]) {
	      printf("RCU[%02d].wg=[freq=%6d, phase=%3d, ampl=%3d, nof_samples=%6d, mode=%3d]\n",
		     rcuout,
		     ack.settings()(rcuin).freq,
		     ack.settings()(rcuin).phase,
		     ack.settings()(rcuin).ampl,
		     ack.settings()(rcuin).nof_samples,
		     ack.settings()(rcuin).mode);
	      rcuin++;
	    }
	  }
	} else {
	  cerr << "Error: RSP_GETWG command failed." << endl;
	}
      }
      break;

    case RSP_SETWGACK:
      {
	RSPSetwgackEvent ack(e);
	
	if (SUCCESS != ack.status) {
	  cerr << "Error: RSP_SETWG command failed." << endl;
	}
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  GCFTask::stop();

  return status;
}

StatusCommand::StatusCommand()
{
}

void StatusCommand::send(GCFPortInterface& /*port*/)
{
  cerr << "Error: option '--status' not supported yet." << endl;
  exit(EXIT_FAILURE);
}

GCFEvent::TResult StatusCommand::ack(GCFEvent& /*e*/)
{
  return GCFEvent::NOT_HANDLED;
}

StatisticsCommand::StatisticsCommand() : m_type(Statistics::SUBBAND_POWER)
{
}

void StatisticsCommand::send(GCFPortInterface& port)
{
  if (getMode()) {
#if 0
    // GET
    RSPGetstatsEvent getstats;

    getstats.timestamp = Timestamp(0,0);
    getstats.rcumask = getRCUMask();
    getstats.cache = true;
    getstats.type = m_type;

    port.send(getstats);
#else
    // SUBSCRIBE
    RSPSubstatsEvent substats;

    substats.timestamp = Timestamp(0,0);
    substats.rcumask = getRCUMask();
    substats.period = 1;
    substats.type = m_type;
    substats.reduction = SUM;

    port.send(substats);
#endif
  } else {
    // SET 
    cerr << "Error: set mode not support for option '--statistics'" << endl;
    GCFTask::stop();
  }
}

void StatisticsCommand::plot_statistics(Array<double, 2>& stats, const Timestamp& timestamp)
{
  static gnuplot_ctrl* handle = 0;
  int n_freqbands = stats.extent(secondDim);
  bitset<MAX_N_RCUS> mask = getRCUMask();

  // initialize the freq array
  firstIndex i;
  
  if (!handle)
  {
    handle = gnuplot_init();
    if (!handle) return;

    //gnuplot_setstyle(handle, "steps");
    gnuplot_cmd(handle, "set grid x y");
  }

  //gnuplot_cmd(handle, "set xlabel \"Frequency (MHz)\" 0, 1.5");
  gnuplot_cmd(handle, "set xlabel \"Frequency (MHz)\"");
  gnuplot_cmd(handle, "set ylabel \"dB\"");
  gnuplot_cmd(handle, "set yrange [0:140]");

  switch (m_type)
    {
    case Statistics::SUBBAND_POWER:
      gnuplot_cmd(handle, "set xrange [0:%f]", SAMPLE_FREQUENCY / 2.0);
      break;
    case Statistics::BEAMLET_POWER:
      gnuplot_cmd(handle, "set xrange [0:%d]", MEPHeader::N_BEAMLETS);
      break;
    }

  char plotcmd[2048];
  time_t seconds = timestamp.sec();
  strftime(plotcmd, 255, "set title \"%s - %a, %d %b %Y %H:%M:%S  %z\"", gmtime(&seconds));

  gnuplot_cmd(handle, plotcmd);

  strcpy(plotcmd, "plot ");

  int count = 0;
  for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {
    if (mask[rcuout]) {
      if (count > 0) strcat(plotcmd, ",");
      count++;

      switch (m_type)
	{
	case Statistics::SUBBAND_POWER:
	  snprintf(plotcmd + strlen(plotcmd), 128, "\"-\" using (%.1f/%.1f*$1):(10*log10($2)) title \"(RCU=%d)\" with steps ",
		   SAMPLE_FREQUENCY, n_freqbands*2.0, rcuout);
	  break;
	case Statistics::BEAMLET_POWER:
	  //snprintf(plotcmd + strlen(plotcmd), 128, "\"-\" using (%.1f/%.1f*$1):(10*log10($2)) title \"Beamlet Power (RSP board=%d)\" with steps ",
	  //SAMPLE_FREQUENCY, n_freqbands*2.0, rcuout);
	  snprintf(plotcmd + strlen(plotcmd), 128, "\"-\" using (1.0*$1):(10*log10($2)) title \"Beamlet Power (RSP board=%d)\" with steps ", rcuout);
	  break;
	default:
	  cerr << "Error: invalid m_type" << endl;
	  exit(EXIT_FAILURE);
	  break;
	}
    }
  }

  gnuplot_cmd(handle, plotcmd);

  gnuplot_write_matrix(handle, stats);

  //gnuplot_cmd(handle, "set nomultiplot");
}

GCFEvent::TResult StatisticsCommand::ack(GCFEvent& e)
{
  if (e.signal == RSP_SUBSTATSACK)
    {
      RSPSubstatsackEvent ack(e);

      if (SUCCESS != ack.status) {
	cerr << "failed to subscribe to statistics" << endl;
	exit(EXIT_FAILURE);
      }

      return GCFEvent::HANDLED;
    }

  if (e.signal != RSP_UPDSTATS) return GCFEvent::NOT_HANDLED;

#if 0
  RSPGetstatsackEvent ack(e);
  bitset<MAX_N_RCUS> mask = getRCUMask();

  if (SUCCESS == ack.status) {
    int rcuin = 0;
    for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {
	
      if (mask[rcuout]) {
	printf("RCU[%02d].statistics=\n", rcuout);
	for (int subband = 0; subband < ack.stats().extent(secondDim); subband++) {
	  printf("%20.10E\n", ack.stats()(rcuin, subband));
	}
	rcuin++;
      }
    }

    plot_statistics(ack.stats());
  } else {
    cerr << "Error: RSP_GETSTATS command failed." << endl;
  }
  GCFTask::stop();

#else
  RSPUpdstatsEvent upd(e);

  if (SUCCESS == upd.status) {
    plot_statistics(upd.stats(), upd.timestamp);
  }
#endif

  return GCFEvent::HANDLED;
}

XCStatisticsCommand::XCStatisticsCommand()
{
}

void XCStatisticsCommand::send(GCFPortInterface& port)
{
  if (getMode()) {
#if 0
    // GET
    RSPGetxcstatsEvent getxcstats;

    getxcstats.timestamp = Timestamp(0,0);
    getxcstats.rcumask = getRCUMask();
    getxcstats.cache = true;
    getxcstats.type = m_type;

    port.send(getxcstats);
#else
    // SUBSCRIBE
    RSPSubxcstatsEvent subxcstats;

    subxcstats.timestamp = Timestamp(0,0);
    subxcstats.rcumask = getRCUMask();
    subxcstats.period = 1;

    port.send(subxcstats);
#endif
  } else {
    // SET 
    cerr << "Error: set mode not support for option '--xcstatistics'" << endl;
    GCFTask::stop();
  }
}

void XCStatisticsCommand::plot_xcstatistics(Array<complex<double>, 4>& xcstats, const Timestamp& timestamp)
{
  LOG_INFO_STR("plot_xcstatistics (shape=" << xcstats.shape() << ") @ " << timestamp);
  LOG_INFO_STR("XX.real()=" << real(xcstats(0,0,Range::all(),Range::all())));
  LOG_INFO_STR("XX.imag()=" << imag(xcstats(0,0,Range::all(),Range::all())));
  LOG_INFO_STR("XY.real()=" << real(xcstats(0,1,Range::all(),Range::all())));
  LOG_INFO_STR("XY.imag()=" << imag(xcstats(0,1,Range::all(),Range::all())));
  LOG_INFO_STR("YX.real()=" << real(xcstats(1,0,Range::all(),Range::all())));
  LOG_INFO_STR("YX.imag()=" << imag(xcstats(1,0,Range::all(),Range::all())));
  LOG_INFO_STR("YY.real()=" << real(xcstats(1,1,Range::all(),Range::all())));
  LOG_INFO_STR("YY.imag()=" << imag(xcstats(1,1,Range::all(),Range::all())));

  xcstats(0,0,2,2) = 1.0;

  static gnuplot_ctrl* handle = 0;
  int n_ant = xcstats.extent(thirdDim);

  //bitset<MAX_N_RCUS> mask = getRCUMask();

  // initialize the freq array
  firstIndex i;
  
  if (!handle)
  {
    handle = gnuplot_init();
    if (!handle) return;

    //gnuplot_setstyle(handle, "steps");
    gnuplot_cmd(handle, "set grid x y");
  }

  //gnuplot_cmd(handle, "set xlabel \"Frequency (MHz)\" 0, 1.5");
  gnuplot_cmd(handle, "set xlabel \"RCU\"");
  gnuplot_cmd(handle, "set ylabel \"RCU\"");
  gnuplot_cmd(handle, "set xrange [0:%d]", n_ant-1);
  gnuplot_cmd(handle, "set yrange [0:%d]", n_ant-1);

  char plotcmd[2048];
  time_t seconds = timestamp.sec();
  strftime(plotcmd, 255, "set title \"%s - %a, %d %b %Y %H:%M:%S  %z\"", gmtime(&seconds));

  gnuplot_cmd(handle, plotcmd);

  gnuplot_cmd(handle,
	      "set view 0,0\n"
	      "set ticslevel 0\n"
	      "unset xtics\n"
	      "unset ytics\n"
	      "unset colorbox\n"
	      "set key off\n"
	      "set border 0\n");

  gnuplot_cmd(handle, "splot \"-\" matrix with points ps 12 pt 5 palette");

  gnuplot_write_matrix(handle, real(xcstats(0,0,Range::all(),Range::all())), true);
}

GCFEvent::TResult XCStatisticsCommand::ack(GCFEvent& e)
{
  if (e.signal == RSP_SUBXCSTATSACK)
    {
      RSPSubxcstatsackEvent ack(e);

      if (SUCCESS != ack.status) {
	cerr << "failed to subscribe to xcstatistics" << endl;
	exit(EXIT_FAILURE);
      }

      return GCFEvent::HANDLED;
    }

  if (e.signal != RSP_UPDXCSTATS) return GCFEvent::NOT_HANDLED;

#if 0
  RSPGetstatsackEvent ack(e);
  bitset<MAX_N_RCUS> mask = getRCUMask();

  if (SUCCESS == ack.status) {
    int rcuin = 0;
    for (int rcuout = 0; rcuout < get_nrcus(); rcuout++) {
	
      if (mask[rcuout]) {
	printf("RCU[%02d].statistics=\n", rcuout);
	for (int subband = 0; subband < ack.stats().extent(secondDim); subband++) {
	  printf("%20.10E\n", ack.stats()(rcuin, subband));
	}
	rcuin++;
      }
    }

    plot_statistics(ack.stats());
  } else {
    cerr << "Error: RSP_GETSTATS command failed." << endl;
  }
  GCFTask::stop();

#else
  RSPUpdxcstatsEvent upd(e);

  if (SUCCESS == upd.status) {
    plot_xcstatistics(upd.stats(), upd.timestamp);
  }
#endif

  return GCFEvent::HANDLED;
}

VersionCommand::VersionCommand()
{
}

void VersionCommand::send(GCFPortInterface& port)
{
  RSPGetversionEvent getversion;

  getversion.timestamp = Timestamp(0,0);
  getversion.cache = true;

  port.send(getversion);
}

GCFEvent::TResult VersionCommand::ack(GCFEvent& e)
{
  RSPGetversionackEvent ack(e);

  if (SUCCESS == ack.status) {
    for (int rsp=0; rsp < GET_CONFIG("RS.N_RSPBOARDS", i); rsp++) {
      printf("RSP[%02d].version=rsp:0x%02x bp:0x%02x ap:0x%02x\n",
	     rsp,
	     ack.versions.rsp()(rsp),
	     ack.versions.bp()(rsp),
	     ack.versions.ap()(rsp));
    }
  } else {
    cerr << "Error: RSP_GETVERSION command failed." << endl;
  }
  GCFTask::stop();

  return GCFEvent::HANDLED;
}

RSPCtl::RSPCtl(string name, Command& command)
  : GCFTask((State)&RSPCtl::initial, name), m_command(command)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

RSPCtl::~RSPCtl()
{
}

GCFEvent::TResult RSPCtl::initial(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(RSPCtl::docommand);
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

GCFEvent::TResult RSPCtl::docommand(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	m_command.send(m_server);
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();
	cout << "Error: port '" << port.getName() << "' disconnected." << endl;
	exit(EXIT_FAILURE);
      }
      break;

    case RSP_GETRCUACK:
    case RSP_SETRCUACK:
    case RSP_GETSTATSACK:
    case RSP_SUBSTATSACK:
    case RSP_UPDSTATS:
    case RSP_SUBXCSTATSACK:
    case RSP_UPDXCSTATS:
    case RSP_GETVERSIONACK:
    case RSP_GETSUBBANDSACK:
    case RSP_SETSUBBANDSACK:
    case RSP_SETWEIGHTSACK:
    case RSP_GETWEIGHTSACK:
    case RSP_GETWGACK:
    case RSP_SETWGACK:
      status = m_command.ack(e); // handle the acknowledgement
      break;

    default:
      cerr << "Error: unhandled event." << endl;
      GCFTask::stop();
      break;
    }

  return status;
}

void RSPCtl::mainloop()
{
  start(); // make initial transition
  GCFTask::run();
}

static void usage()
{
  cout << "rspctl usage:" << endl;
  cout << endl;
  cout << "rspctl --weights        [--select=<set>] # get weights" << endl;
  cout << "  Example --select sets: --select=1,2,4:7 or --select=1:3,5:7" << endl;
  cout << "rspctl --weights=(0|1)  [--select=<set>] # set weights" << endl;
  cout << "rspctl --subbands       [--select=<set>] # get subband selection" << endl;
  cout << "rspctl --subbands=<set> [--select=<set>] # set subband selection" << endl;
  cout << "  Example --subbands sets: --subbands=0:39 or --select=0:19,40:59" << endl;
  cout << "rspctl --rcu            [--select=<set>] # get RCU control" << endl;
  cout << "rspctl --rcu=0x??       [--select=<set>] # set RCU control" << endl;
  cout << "             0x80 = VDDVCC_ENABLE" << endl;
  cout << "             0x40 = VH_ENABLE" << endl;
  cout << "             0x20 = VL_ENABLE" << endl;
  cout << "             0x10 = FILSEL_1" << endl;
  cout << "             0x08 = FILSEL_0" << endl;
  cout << "             0x04 = BANDSEL" << endl;
  cout << "             0x02 = HBA_ENABLE" << endl;
  cout << "             0x01 = LBA_ENABLE" << endl;
  cout << "  Common values: LB_10_90=0xB9, HB_110_190=0xC6, HB_170_230=0xCE, HB_210_250=0xD6" << endl;
  cout << "rspctl --wg             [--select=<set>] # get waveform generator settings" << endl;
  cout << "rspctl --wg=freq        [--select=<set>] # set waveform generator settings" << endl;
  cout << "rspctl --status         [--select=<set>] # get status" << endl;
  cout << "rspctl --statistics[=(subband|beamlet)]" << endl;
  cout << "                        [--select=<set>] # get subband (default) or beamlet statistics" << endl;
  cout << "rspctl --version        [--select=<set>] # get version information" << endl;
}

static Command* parse_options(int argc, char** argv)
{
  Command *command = 0;
  set<int> select;

  //
  // --select defaults to all
  //
  for (int i = 0; i < GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL; i++) {
    select.insert(i);
  }

  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  while (1)
    {
      static struct option long_options[] = 
	{
	  { "select",     required_argument, 0, 'l' },
	  { "weights",    optional_argument, 0, 'w' },
	  { "subbands",   optional_argument, 0, 's' },
	  { "rcu",        optional_argument, 0, 'r' },
	  { "wg",         optional_argument, 0, 'g' },
	  { "status",     no_argument,       0, 'q' },
	  { "statistics", optional_argument, 0, 't' },
	  { "xcstatistics", no_argument,     0, 'x' },
	  { "version",    no_argument,       0, 'v' },
	  { "help",       no_argument,       0, 'h' },
	  
	  { 0, 0, 0, 0 },
	};

      int option_index = 0;
      int c = getopt_long(argc, argv,
			  "l:w::s::r::g::qt::vh", long_options, &option_index);
    
      if (c == -1) break;
    
      switch (c)
	{
	case 'l':
	  if (optarg) {
	    select = strtoset(optarg,
			      GET_CONFIG("RS.N_BLPS", i) *
			      GET_CONFIG("RS.N_RSPBOARDS", i) *
			      MEPHeader::N_POL);
	    if (select.empty()) {
	      cerr << "Error: invalid or missing '--select' option" << endl;
	      exit(EXIT_FAILURE);
	    }
	  } else {
	    cerr << "Error: option '--select' requires an argument" << endl;
	  }
	  break;

	case 'w':
	  {
	    if (command) delete command;
	    WeightsCommand* weightscommand = new WeightsCommand();
	    command = weightscommand;
	    
	    if (optarg) {
	      weightscommand->setMode(false);
	      double value = atof(optarg);
	      weightscommand->setValue(value);
	      
	      if (value <= -1.0 || value > 1.0) {
		cerr << "Error: invalid weights value, should be: -1 < value <= 1" << endl;
		exit(EXIT_FAILURE);
	      }
	    }
	  }
	  break;

	case 's':
	  {
	    if (command) delete command;
	    SubbandsCommand* subbandscommand = new SubbandsCommand();
	    command = subbandscommand;
	    
	    if (optarg) {
	      subbandscommand->setMode(false);
	      set<int> subbandset = strtoset(optarg, MEPHeader::N_SUBBANDS);
	      if (subbandset.empty()) {
		cerr << "Error: invalid or empty '--subbands' option" << endl;
		exit(EXIT_FAILURE);
	      }
	      subbandscommand->setSubbandSet(subbandset);
	    }
	  }
	  break;

	case 'r':
	  {
	    if (command) delete command;
	    RCUCommand* rcucommand = new RCUCommand();
	    command = rcucommand;
	    
	    if (optarg) {
	      rcucommand->setMode(false);
	      unsigned long controlopt = strtoul(optarg, 0, 0);
	      if ( controlopt > 0xFF )
		{
		  cerr << "Error: option '--rcu' parameter must be < 0xFF" << endl;
		  delete command;
		  return 0;
		}
	      rcucommand->setControl(controlopt);
	    }
	  }
	  break;

	case 'g':
	  {
	    if (command) delete command;
	    WGCommand* wgcommand = new WGCommand();
	    command = wgcommand;
	    
	    if (optarg) {
	      wgcommand->setMode(false);
	      double frequency = atof(optarg);
	      if ( frequency < 0 )
		{
		  cerr << "Error: option '--wg' parameter must be > 0" << endl;
		  delete command;
		  return 0;
		}
	      wgcommand->setFrequency(frequency);
	    }
	  }
	  break;

	case 'q':
	  {
	    if (command) delete command;
	    StatusCommand* statuscommand = new StatusCommand();
	    command = statuscommand;
	  }
	  break;

	case 't':
	  {
	    if (command) delete command;
	    StatisticsCommand* statscommand = new StatisticsCommand();
	    command = statscommand;

	    if (optarg) {
	      if (!strcmp(optarg, "subband")) {
		statscommand->setType(Statistics::SUBBAND_POWER);
	      } else if (!strcmp(optarg, "beamlet")) {
		statscommand->setType(Statistics::BEAMLET_POWER);

		// default for beamlet stats select is N_RSPBOARDS * N_POL
		select.clear();
		for (int i = 0; i < GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL; i++) {
		  select.insert(i);
		}
	      }
	    }
	  }
	  break;

	case 'x':
	  {
	    if (command) delete command;
	    XCStatisticsCommand* xcstatscommand = new XCStatisticsCommand();
	    command = xcstatscommand;
	  }
	  break;

	case 'v':
	  {
	    if (command) delete command;
	    VersionCommand* versioncommand = new VersionCommand();
	    command = versioncommand;
	  }
	  break;

	case 'h':
	  usage();
	  break;

	case '?':
	  exit(EXIT_FAILURE);
	  break;
	}
    }
  
  if (command) {
    command->setSelect(select);
    command->set_nrcus(GET_CONFIG("RS.N_BLPS", i) *
		       GET_CONFIG("RS.N_RSPBOARDS", i) *
		       MEPHeader::N_POL);
  }

  return command;
}

int main(int argc, char** argv)
{
  Command* command = 0;

  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try
  {
    GCF::ParameterSet::instance()->adoptFile(RSP_SYSCONF "/RemoteStation.conf");
  }
  catch (Exception e)
  {
    cout << "500 Error: failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  if (0 == (command = parse_options(argc, argv))) {
    cout << "Warning: no command specified." << endl;
    exit(EXIT_FAILURE);
  }

  RSPCtl c("RSPCtl", *command);
  
  try
  {
    c.mainloop();
  }
  catch (Exception e)
  {
    cout << "500 Error: exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  delete command;
  LOG_INFO("Normal termination of program");

  return 0;
}
