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
//# rspctl --weights         [--select=<set>]  # get weights
//# rspctl --weights=(0|1)   [--select=<set>]  # set weights
//# rspctl --subbands        [--select=<set>]  # get subband selection
//# rspctl --subbands=<list> [--select=<set>]  # set subband selection
//# rspctl --rcu             [--select=<set>]  # get RCU control
//# rspctl --rcu=0xce        [--select=<set>]  # set RCU control
//# rspctl --wg              [--select=<set>]  # get waveform generator settings
//# rspctl --wg=freq         [--select=<set>]  # set wg freq is in Hz (float)
//# rspctl --status          [--select=<set>]  # get status
//# rspctl --statistics[=(subband|beamlet)]
//#                          [--select=<set>]  # get subband (default) or beamlet statistics
//# rspctl --xcstatistics    [--select=<set>]  # get cross correlation statistics
//# rspctl --xcsubband=<int>                   # set the subband to cross correlate
//# rspctl --clocks          [--select=<set>]  # get or set the clock frequency of clocks
//# rspctl --version         [--select=<set>]  # get version information
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
#include <complex>

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

//
// Sample frequency, should be queried from the RSPDriver
//
#define SAMPLE_FREQUENCY 160.0e6

static std::list<int> strtolist(const char* str, int max)
{
  string inputstring(str);
  char* start = (char*)inputstring.c_str();
  char* end   = 0;
  bool  range = false;
  long prevval = 0;
  list<int> resultset;

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
            if (0 == prevval && 0 == val)
              val = max - 1;
            if (val < prevval)
            {
              cerr << "Error: invalid range specification" << endl;
              resultset.clear();
              return resultset;
            }
            for (long i = prevval; i <= val; i++)
              resultset.push_back(i);
          }

          else
          {
            resultset.push_back(val);
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
  if (getMode())
  {
    // GET
    RSPGetweightsEvent getweights;

    getweights.timestamp = Timestamp(0,0);
    getweights.rcumask = getRCUMask();
    getweights.cache = true;

    port.send(getweights);
  }
  else
  {
    // SET
    RSPSetweightsEvent setweights;
    setweights.timestamp = Timestamp(0,0);
    setweights.rcumask = getRCUMask();

    cerr << "rcumask.count()=" << setweights.rcumask.count() << endl;

    setweights.weights().resize(1, setweights.rcumask.count(), MEPHeader::N_BEAMLETS);

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

      if (SUCCESS == ack.status)
      {
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
            printf("RCU[%02d].weights=", rcuout);
            cout << ack.weights()(0, rcuin++, Range::all()) << endl;
          }
        }
      }
      else
      {
        cerr << "Error: RSP_GETWEIGHTS command failed." << endl;
      }
    }
    break;

    case RSP_SETWEIGHTSACK:
    {
      RSPSetweightsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
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
  if (getMode())
  {
    // GET
    RSPGetsubbandsEvent getsubbands;

    getsubbands.timestamp = Timestamp(0,0);
    getsubbands.rcumask = getRCUMask();
    getsubbands.cache = true;

    port.send(getsubbands);
  }
  else
  {
    // SET
    RSPSetsubbandsEvent setsubbands;
    setsubbands.timestamp = Timestamp(0,0);
    setsubbands.rcumask = getRCUMask();

    cerr << "rcumask.count()=" << setsubbands.rcumask.count() << endl;

    setsubbands.subbands().resize(1, m_subbandlist.size());

    int i = 0;
    std::list<int>::iterator it;
    for (it = m_subbandlist.begin(); it != m_subbandlist.end(); it++, i++)
    {
      if (i >= MEPHeader::N_SUBBANDS)
        break;
      setsubbands.subbands()(0, i) = (*it);
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

      if (SUCCESS == ack.status)
      {
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
            printf("RCU[%02d].subbands=", rcuout);
            cout << ack.subbands()(rcuin++, Range::all()) << endl;
          }
        }
      }
      else
      {
        cerr << "Error: RSP_GETSUBBANDS command failed." << endl;
      }
    }
    break;

    case RSP_SETSUBBANDSACK:
    {
      RSPSetsubbandsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
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
  if (getMode())
  {
    // GET
    RSPGetrcuEvent getrcu;

    getrcu.timestamp = Timestamp(0,0);
    getrcu.rcumask = getRCUMask();
    getrcu.cache = true;

    port.send(getrcu);
  }
  else
  {
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

      if (SUCCESS == ack.status)
      {
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
            printf("RCU[%02d].status=0x%02x\n",
                   rcuout, ack.settings()(rcuin++).value);
          }
        }
      }
      else
      {
        cerr << "Error: RSP_GETRCU command failed." << endl;
      }
    }
    break;

    case RSP_SETRCUACK:
    {
      RSPSetrcuackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        cerr << "Error: RSP_SETRCU command failed." << endl;
      }
    }
  }

  GCFTask::stop();

  return GCFEvent::HANDLED;
}

ClocksCommand::ClocksCommand()
{
}

void ClocksCommand::send(GCFPortInterface& port)
{
  if (getMode())
  {
    // GET
    RSPGetclocksEvent getclocks;

    getclocks.timestamp = Timestamp(0,0);
    getclocks.tdmask = getTDMask();
    getclocks.cache = true;

    port.send(getclocks);
  }
  else
  {
    // SET
    RSPSetclocksEvent setclocks;
    setclocks.timestamp = Timestamp(0,0);

    setclocks.tdmask = getTDMask();
    setclocks.clocks().resize(1);
    setclocks.clocks()(0) = m_clock;

    port.send(setclocks);
  }
}

GCFEvent::TResult ClocksCommand::ack(GCFEvent& e)
{
  switch (e.signal)
  {
    case RSP_GETCLOCKSACK:
    {
      RSPGetclocksackEvent ack(e);
      bitset<MAX_N_TDS> mask = getTDMask();

      if (SUCCESS == ack.status)
      {
        int tdin = 0;
        for (int tdout = 0; tdout < get_ndevices(); tdout++)
        {

          if (mask[tdout])
          {
            printf("TD[%02d] clock=%d\n",tdout, ack.clocks()(tdin++));
          }
        }
      }
      else
      {
        cerr << "Error: RSP_GETCLOCKS command failed." << endl;
      }
    }
    break;

    case RSP_SETCLOCKSACK:
    {
      RSPSetclocksackEvent ack(e);

      if (SUCCESS != ack.status)
      {
        cerr << "Error: RSP_SETCLOCKS command failed." << endl;
      }
    }
  }

  GCFTask::stop();

  return GCFEvent::HANDLED;
}

WGCommand::WGCommand() :
    m_frequency(0.0),
    m_phase(0),
    m_amplitude(64)
{
}

void WGCommand::send(GCFPortInterface& port)
{
  if (getMode())
  {
    // GET
    RSPGetwgEvent wgget;
    wgget.timestamp = Timestamp(0,0);
    wgget.rcumask = getRCUMask();
    wgget.cache = true;
    port.send(wgget);

  }
  else
  {
    // SET
    RSPSetwgEvent wgset;

    wgset.timestamp = Timestamp(0,0);
    wgset.rcumask = getRCUMask();
    wgset.settings().resize(1);
    wgset.settings()(0).freq = (uint16)(((m_frequency * (1<<16)) / SAMPLE_FREQUENCY) + 0.5);
    wgset.settings()(0).phase = m_phase;
    wgset.settings()(0).ampl = m_amplitude;
    wgset.settings()(0).nof_samples = N_WAVE_SAMPLES;

    if (m_frequency > 1e-6)
    {
      wgset.settings()(0).mode = WGSettings::MODE_CALC;
    }
    else
    {
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

      if (SUCCESS == ack.status)
      {

        // print settings
        bitset<MAX_N_RCUS> mask = getRCUMask();
        int rcuin = 0;
        for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
        {

          if (mask[rcuout])
          {
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
      }
      else
      {
        cerr << "Error: RSP_GETWG command failed." << endl;
      }
    }
    break;

    case RSP_SETWGACK:
    {
      RSPSetwgackEvent ack(e);

      if (SUCCESS != ack.status)
      {
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
  if (getMode())
  {
    // SUBSCRIBE
    RSPSubstatsEvent substats;

    substats.timestamp = Timestamp(0,0);
    substats.rcumask = getRCUMask();
    substats.period = 1;
    substats.type = m_type;
    substats.reduction = SUM;

    port.send(substats);
  }
  else
  {
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
    if (!handle)
      return;

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
  for (int rcuout = 0; rcuout < get_ndevices(); rcuout++)
  {
    if (mask[rcuout])
    {
      if (count > 0)
        strcat(plotcmd, ",");
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

    if (SUCCESS != ack.status)
    {
      cerr << "failed to subscribe to statistics" << endl;
      exit(EXIT_FAILURE);
    }

    return GCFEvent::HANDLED;
  }

  if (e.signal != RSP_UPDSTATS)
    return GCFEvent::NOT_HANDLED;

  RSPUpdstatsEvent upd(e);

  if (SUCCESS == upd.status)
  {
    plot_statistics(upd.stats(), upd.timestamp);
  }

  return GCFEvent::HANDLED;
}

XCStatisticsCommand::XCStatisticsCommand()
{
}

void XCStatisticsCommand::send(GCFPortInterface& port)
{
  if (getMode())
  {
    // SUBSCRIBE
    RSPSubxcstatsEvent subxcstats;

    subxcstats.timestamp = Timestamp(0,0);
    subxcstats.rcumask = getRCUMask();
    subxcstats.period = 1;

    port.send(subxcstats);
  }
  else
  {
    // SET
    cerr << "Error: set mode not support for option '--xcstatistics'" << endl;
    GCFTask::stop();
  }
}

void XCStatisticsCommand::plot_xcstatistics(Array<complex<double>, 4>& xcstats, const Timestamp& timestamp)
{
  LOG_INFO_STR("plot_xcstatistics (shape=" << xcstats.shape() << ") @ " << timestamp);
  //LOG_INFO_STR("XX()=" << xcstats(0,0,Range::all(),Range::all()));
  //LOG_INFO_STR("XY()=" << xcstats(0,1,Range::all(),Range::all()));
  //LOG_INFO_STR("YX()=" << xcstats(1,0,Range::all(),Range::all()));
  //LOG_INFO_STR("YY()=" << xcstats(1,1,Range::all(),Range::all()));

  //xcstats(0,0,2,2) = 1.0;

  static gnuplot_ctrl* handle = 0;
  int n_ant = xcstats.extent(thirdDim);

  //bitset<MAX_N_RCUS> mask = getRCUMask();

  // initialize the freq array
  firstIndex i;

  if (!handle)
  {
    handle = gnuplot_init();
    if (!handle)
      return;

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

  //xcstats = 10.0*log(real(xcstats)*real(xcstats))/log(10.0);

  gnuplot_write_matrix(handle, real(xcstats(0,0,Range::all(),Range::all())), true);
}

/**
 * Function to convert the complex semi-floating point representation used by the
 * EPA firmware to a complex<double>.
 */
BZ_DECLARE_FUNCTION_RET(convert_to_powerangle, complex<double>)
inline complex<double> convert_to_powerangle(complex<double> val)
{
  double angle = 0.0;
  double power = real(val)*real(val) + imag(val)*imag(val);

  if (power > 0.0)
  {
    power = 12 + 5*log10(power); // adjust scaling to allow comparison to subband statistics
  }

  if (0.0 == real(val))
  {

    if (imag(val) > 0)
      angle = 90.0;
    else if (imag(val) < 0)
      angle = 270;

  }
  else
  {

    angle = 45.0 * atan(imag(val)/real(val)) / atan(1.0);

    if (real(val) > 0.0)
    {
      if (imag(val) < 0)
        angle += 360.0;
    }
    else
      angle += 180.0;

  }

  return complex<double>(power, angle);
}

GCFEvent::TResult XCStatisticsCommand::ack(GCFEvent& e)
{
  if (e.signal == RSP_SUBXCSTATSACK)
  {
    RSPSubxcstatsackEvent ack(e);

    if (SUCCESS != ack.status)
    {
      cerr << "failed to subscribe to xcstatistics" << endl;
      exit(EXIT_FAILURE);
    }

    return GCFEvent::HANDLED;
  }

  if (e.signal != RSP_UPDXCSTATS)
    return GCFEvent::NOT_HANDLED;

  RSPUpdxcstatsEvent upd(e);

  if (SUCCESS == upd.status)
  {
    upd.stats() = convert_to_powerangle(upd.stats());

    for (int i = 0; i < upd.stats().extent(firstDim) * upd.stats().extent(thirdDim); i++)
    {
      for (int j = 0; j < upd.stats().extent(secondDim) * upd.stats().extent(fourthDim); j++)
      {
        printf("%3.0f:%03.0f ", real(upd.stats()(i%2,j%2,i/2,j/2)), imag(upd.stats()(i%2,j%2,i/2,j/2)));
      }
      printf("\n");
    }

    plot_xcstatistics(upd.stats(), upd.timestamp);
  }

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

  if (SUCCESS == ack.status)
  {
    for (int rsp=0; rsp < get_ndevices(); rsp++)
    {
      printf("RSP[%02d].version=rsp:0x%02x bp:0x%02x ap:0x%02x\n",
             rsp,
             ack.versions.rsp()(rsp),
             ack.versions.bp()(rsp),
             ack.versions.ap()(rsp));
    }
  }
  else
  {
    cerr << "Error: RSP_GETVERSION command failed." << endl;
  }
  GCFTask::stop();

  return GCFEvent::HANDLED;
}

RSPCtl::RSPCtl(string name, int argc, char** argv)
    : GCFTask((State)&RSPCtl::initial, name), m_command(0),
    m_nrcus(0), m_nrspboards(0), m_ntdboards(0), m_argc(argc), m_argv(argv)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

RSPCtl::~RSPCtl()
{
  if (m_command)
    delete m_command;
}

GCFEvent::TResult RSPCtl::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
    case F_INIT:
    break;

    case F_ENTRY:
    {
      if (!m_server.isConnected())
        m_server.open();
    }
    break;

    case F_CONNECTED:
    {
      if (m_server.isConnected())
      {
        RSPGetconfigEvent getconfig;
        m_server.send(getconfig);
      }
    }
    break;

    case RSP_GETCONFIGACK:
    {
      RSPGetconfigackEvent ack(e);
      m_nrcus = ack.n_rcus;
      m_nrspboards = ack.n_rspboards;
      m_ntdboards = ack.n_tdboards;
      LOG_DEBUG_STR("n_rcus     =" << m_nrcus);
      LOG_DEBUG_STR("n_rspboards=" << m_nrspboards);
      LOG_DEBUG_STR("n_tdboards =" << m_ntdboards);
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
      if (0 == (m_command = parse_options(m_argc, m_argv)))
      {
        cout << "Warning: no command specified." << endl;
        exit(EXIT_FAILURE);
      }

      m_command->send(m_server);
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
    case RSP_SETCLOCKSACK:
    case RSP_GETCLOCKSACK:
      status = m_command->ack(e); // handle the acknowledgement
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
  cout << "rspctl --weights        [--select=<set>]  # get weights" << endl;
  cout << "  Example --select sets: --select=1,2,4:7 or --select=1:3,5:7" << endl;
  cout << "rspctl --weights=(0|1)  [--select=<set>]  # set weights" << endl;
  cout << "rspctl --subbands       [--select=<set>]  # get subband selection" << endl;
  cout << "rspctl --subbands=<set> [--select=<set>]  # set subband selection" << endl;
  cout << "  Example --subbands sets: --subbands=0:39 or --select=0:19,40:59" << endl;
  cout << "rspctl --rcu            [--select=<set>]  # get RCU control" << endl;
  cout << "rspctl --rcu=0x??       [--select=<set>]  # set RCU control" << endl;
  cout << "             0x80 = VDDVCC_ENABLE" << endl;
  cout << "             0x40 = VH_ENABLE" << endl;
  cout << "             0x20 = VL_ENABLE" << endl;
  cout << "             0x10 = FILSEL_1" << endl;
  cout << "             0x08 = FILSEL_0" << endl;
  cout << "             0x04 = BANDSEL" << endl;
  cout << "             0x02 = HBA_ENABLE" << endl;
  cout << "             0x01 = LBA_ENABLE" << endl;
  cout << "  Common values: LB_10_90=0xB9, HB_110_190=0xC6, HB_170_230=0xCE, HB_210_250=0xD6" << endl;
  cout << "rspctl --wg             [--select=<set>]  # get waveform generator settings" << endl;
  cout << "rspctl --wg=freq        [--select=<set>]  # set waveform generator settings" << endl;
  cout << "rspctl --status         [--select=<set>]  # get status" << endl;
  cout << "rspctl --statistics[=(subband|beamlet)]" << endl;
  cout << "                        [--select=<set>]  # get subband (default) or beamlet statistics" << endl;
  cout << "rspctl --xcstatistics   [--select=<set>]  # get crosscorrelation statistics" << endl;
  cout << "rspctl --xcsubband=<int>                  # set the subband to cross correlate" << endl;
  cout << "rspctl --clocks=<int>    [--select=<set>] # get or set the clock frequency of clocks in Hz" << endl;
  cout << "rspctl --version         [--select=<set>] # get version information" << endl;
}

Command* RSPCtl::parse_options(int argc, char** argv)
{
  Command *command = 0;
  list<int> select;

  // select all by default
  select.clear();
  for (int i = 0; i < MAX_N_RCUS; ++i)
  {
    select.push_back(i);
  }

  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
  while (1)
  {
    static struct option long_options[] =
      {
        { "select",     required_argument, 0, 'l'
        },
        { "weights",    optional_argument, 0, 'w' },
        { "subbands",   optional_argument, 0, 's' },
        { "rcu",        optional_argument, 0, 'r' },
        { "wg",         optional_argument, 0, 'g' },
        { "status",     no_argument,       0, 'q' },
        { "statistics", optional_argument, 0, 't' },
        { "xcstatistics", no_argument,     0, 'x' },
        { "xcsubband",  required_argument, 0, 'z' },
        { "clocks",     optional_argument, 0, 'c' },
        { "version",    no_argument,       0, 'v' },
        { "help",       no_argument,       0, 'h' },

        { 0, 0, 0, 0 },
      };

    int option_index = 0;
    int c = getopt_long(argc, argv,
                        "l:w::s::r::g::qt::vc::h", long_options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
      case 'l':
        if (optarg)
        {
          if (!command || 0 == command->get_ndevices())
          {
            cerr << "Error: 'command' argument should come before --select argument" << endl;
            exit(EXIT_FAILURE);
          }
          select = strtolist(optarg, command->get_ndevices());
          if (select.empty())
          {
            cerr << "Error: invalid or missing '--select' option" << endl;
            exit(EXIT_FAILURE);
          }
        }
        else
        {
          cerr << "Error: option '--select' requires an argument" << endl;
        }
        break;

      case 'w':
      {
        if (command)
          delete command;
        WeightsCommand* weightscommand = new WeightsCommand();
        command = weightscommand;

        command->set_ndevices(m_nrcus);

        if (optarg)
        {
          weightscommand->setMode(false);
          double value = atof(optarg);
          weightscommand->setValue(value);

          if (value <= -1.0 || value > 1.0)
          {
            cerr << "Error: invalid weights value, should be: -1 < value <= 1" << endl;
            exit(EXIT_FAILURE);
          }
        }
      }
      break;

      case 's':
      {
        if (command)
          delete command;
        SubbandsCommand* subbandscommand = new SubbandsCommand();
        command = subbandscommand;

        command->set_ndevices(m_nrcus);

        if (optarg)
        {
          subbandscommand->setMode(false);
          list<int> subbandlist = strtolist(optarg, MEPHeader::N_SUBBANDS);
          if (subbandlist.empty())
          {
            cerr << "Error: invalid or empty '--subbands' option" << endl;
            exit(EXIT_FAILURE);
          }
          subbandscommand->setSubbandList(subbandlist);
        }
      }
      break;

      case 'r':
      {
        if (command)
          delete command;
        RCUCommand* rcucommand = new RCUCommand();
        command = rcucommand;

        command->set_ndevices(m_nrcus);

        if (optarg)
        {
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
        if (command)
          delete command;
        WGCommand* wgcommand = new WGCommand();
        command = wgcommand;

        command->set_ndevices(m_nrcus);

        if (optarg)
        {
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
        if (command)
          delete command;
        StatusCommand* statuscommand = new StatusCommand();
        command = statuscommand;

        command->set_ndevices(m_nrcus);
      }
      break;

      case 't':
      {
        if (command)
          delete command;
        StatisticsCommand* statscommand = new StatisticsCommand();
        command = statscommand;

        command->set_ndevices(m_nrcus);

        if (optarg)
        {
          if (!strcmp(optarg, "subband"))
          {
            statscommand->setType(Statistics::SUBBAND_POWER);
          }
          else if (!strcmp(optarg, "beamlet"))
          {
            statscommand->setType(Statistics::BEAMLET_POWER);
          }
        }
      }
      break;

      case 'x':
      {
        if (command)
          delete command;
        XCStatisticsCommand* xcstatscommand = new XCStatisticsCommand();
        command = xcstatscommand;
        command->set_ndevices(m_nrcus);
        command->setSelectable(false); // no selection allowed
      }
      break;

      case 'z':
      {
        if (command)
          delete command;
        SubbandsCommand* subbandscommand = new SubbandsCommand();
        command = subbandscommand;

        command->set_ndevices(m_nrcus);

        if (optarg)
        {
          subbandscommand->setMode(false);

          int subband = atoi(optarg);

          if (subband < 0 || subband >= MEPHeader::N_SUBBANDS)
          {
            cerr << "Error: argument to --xcsubband out of range, value must be >= 0 and < " << MEPHeader::N_SUBBANDS << endl;
            exit(EXIT_FAILURE);
          }

          list<int> subbandlist;
          for (int rcu = 0; rcu < m_nrcus / MEPHeader::N_POL; rcu++)
          {
            subbandlist.push_back(subband);
          }
          subbandscommand->setSubbandList(subbandlist);
        }
      }
      break;

      case 'c':
      {
        if (command)
          delete command;
        ClocksCommand* clockcommand = new ClocksCommand();
        command = clockcommand;

        command->set_ndevices(m_ntdboards);

        if (optarg)
        {
          clockcommand->setMode(false);
          double clock = atof(optarg);
          if ( 160000000 != (uint32)clock && 200000000 != (uint32)clock)
          {
            cerr << "Error: option '--clocks' parameter must be 160000000 or 200000000" << endl;
            delete command;
            return 0;
          }
          clockcommand->setClock((uint32)clock);

        }
      }
      break;

      case 'v':
      {
        if (command)
          delete command;
        VersionCommand* versioncommand = new VersionCommand();
        command = versioncommand;
        command->set_ndevices(m_nrspboards);
      }
      break;

      case 'h':
        usage();
        break;

      case '?':
      default:
        exit(EXIT_FAILURE);
        break;
    }
  }

  if (command)
  {
    if (!command->getSelectable())
    {
      // select all
      select.clear();
      for (int i = 0; i < MAX_N_RCUS; ++i)
      {
        select.push_back(i);
      }
    }
    command->setSelect(select);
  }

  return command;
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  RSPCtl c("RSPCtl", argc, argv);

  try
  {
    c.mainloop();
  }
  catch (Exception e)
  {
    cout << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
