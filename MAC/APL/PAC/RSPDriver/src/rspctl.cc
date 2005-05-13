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
//# rspctl --wg=freq        [--select=<set>] # set waveform generator settings
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

using namespace std;
using namespace LOFAR;
using namespace blitz;
using namespace EPA_Protocol;
using namespace RSP_Protocol;
using namespace rspctl;

// local funtions
static void usage();
static Command* parse_options(int argc, char** argv);

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

void WeightsCommand::send(GCFPortInterface& /*port*/)
{
  cerr << "Error: option '--weights' not supported yet." << endl;
  exit(EXIT_FAILURE);
}

GCFEvent::TResult WeightsCommand::ack(GCFEvent& /*ack*/)
{
  return GCFEvent::NOT_HANDLED;
}

SubbandsCommand::SubbandsCommand()
{
}

void SubbandsCommand::send(GCFPortInterface& /*port*/)
{
  cerr << "Error: option '--subbands' not supported yet." << endl;
  exit(EXIT_FAILURE);
}

GCFEvent::TResult SubbandsCommand::ack(GCFEvent& /*ack*/)
{
  return GCFEvent::NOT_HANDLED;
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

WGCommand::WGCommand()
{
}

void WGCommand::send(GCFPortInterface& /*port*/)
{
  cerr << "Error: option '--wg' not supported yet." << endl;
  exit(EXIT_FAILURE);
}

GCFEvent::TResult WGCommand::ack(GCFEvent& /*e*/)
{
  return GCFEvent::NOT_HANDLED;
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
    // GET
    RSPGetstatsEvent getstats;

    getstats.timestamp = Timestamp(0,0);
    getstats.rcumask = getRCUMask();
    getstats.cache = true;
    getstats.type = m_type;

    port.send(getstats);
  } else {
    // SET 
    cerr << "Error: set mode not support for option '--statistics'" << endl;
    GCFTask::stop();
  }
}

GCFEvent::TResult StatisticsCommand::ack(GCFEvent& e)
{
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
  } else {
    cerr << "Error: RSP_GETSTATS command failed." << endl;
  }
  GCFTask::stop();

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
    case RSP_GETVERSIONACK:
      status = m_command.ack(e); // handle the acknowledgement
      break;

    default:
      cerr << "Error: unhandled event." << endl;
      GCFTask::stop();
      break;
    }

  return status;
}

#if 0
GCFEvent::TResult RSPCtl::handlecommand(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      if (XINETD_MODE == m_options.xinetd_mode) {
	cout << "250-Acquiring sensor data." << endl;
	cout << "250-" << endl;
	cout << "250-META sensor = statistics " << m_line << endl;
	cout << "250-" << endl;
      }

      // set rcu control register
      RSPSetrcuEvent setrcu;
      setrcu.timestamp = Timestamp(0,0);
      setrcu.rcumask = m_options.device_set;
      
      setrcu.settings().resize(1);
      setrcu.settings()(0).value = m_options.rcucontrol;

      if (!m_server.send(setrcu))
      {
	cout << "Error: failed to send RCU control" << endl;
	exit(EXIT_FAILURE);
      }
    }
    break;
    
    case RSP_SETRCUACK:
    {
      RSPSetrcuackEvent ack(e);

      if (SUCCESS != ack.status)
      {
	cout << "500 Error: failed to set RCU control register" << endl;
	exit(EXIT_FAILURE);
      }

      sleep(1);

      // subscribe to status updates
      RSPSubstatusEvent substatus;

      substatus.timestamp = Timestamp(0,0);

      substatus.rcumask = m_options.device_set;
      
      substatus.period = 1;
      
      if (!m_server.send(substatus))
      {
	cout << "500 Error: failed to send subscription for status updates" << endl;
	exit(EXIT_FAILURE);
      }

    }
    break;

    case RSP_SUBSTATUSACK:
    {
      RSPSubstatusackEvent ack(e);

      if (SUCCESS != ack.status)
      {
	cout << "500 Error: failed to subscribe to status updates" << endl;
	exit(EXIT_FAILURE);
      }
      m_statushandle = ack.handle;

      // subscribe to statistics updates
      RSPSubstatsEvent substats;

      substats.timestamp = Timestamp(0,0);

      substats.rcumask = m_options.device_set;
      
      substats.period = 1;
      substats.type = m_options.type;
      substats.reduction = SUM;
      
      if (!m_server.send(substats))
      {
	cout << "500 Error: failed to send subscription for statistics updates" << endl;
	exit(EXIT_FAILURE);
      }
    }
    break;

    case RSP_SUBSTATSACK:
    {
      RSPSubstatsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
	cout << "500 Error: failed to subscribe to statistics updates" << endl;
	exit(EXIT_FAILURE);
      }

      m_statshandle = ack.handle;
    }
    break;

    case RSP_UPDSTATUS:
    {
      RSPUpdstatusEvent upd(e);
      
      if (SUCCESS != upd.status)
      {
	cout << "500 Error: invalid update" << endl;
	exit(EXIT_FAILURE);
      }

      switch (m_options.xinetd_mode)
	{
	case CMDLINE_MODE:
	  cout << "time=" << upd.timestamp << endl;
	  break;
	case XINETD_MODE:
	  cout << "250-META time status = " << upd.timestamp << endl;
	  break;
	}

      if (CMDLINE_MODE == m_options.xinetd_mode
	  || XINETD_MODE == m_options.xinetd_mode)
      {
	int result = 0;
	for (int device = 0; device < m_options.n_devices; device++)
	{
	  if (!m_options.device_set[device]) continue;
	  
	  if (XINETD_MODE == m_options.xinetd_mode) {
	    cout << formatString("250-META status, rcu[%02d](status=0x%02x, noverflow=%d)",
				 device,
				 upd.sysstatus.rcu()(result).status,
				 upd.sysstatus.rcu()(result).nof_overflow) << endl;
	  } else {
	    printf("RCU[%02d]:  status=0x%02x  noverflow=%d\n",
		   device,
		   upd.sysstatus.rcu()(result).status,
		   upd.sysstatus.rcu()(result).nof_overflow);
	  }
	  result++;
	}
      }
    }
    break;

    case RSP_UPDSTATS:
    {
      RSPUpdstatsEvent upd(e);

      if (SUCCESS != upd.status)
      {
	cout << "500 Error: invalid update" << endl;
	exit(EXIT_FAILURE);
      }

      switch (m_options.xinetd_mode)
	{
	case CMDLINE_MODE:
	  cout << "time=" << upd.timestamp << endl;
	  break;
	case XINETD_MODE:
	  cout << "250-META time status = " << upd.timestamp << endl;
	  break;
	}

      if (capture_statistics(upd.stats())) {
	if (XINETD_MODE == m_options.xinetd_mode) {
	  m_nseconds = 0; // reset counter
	  cout << "250 Data acquisition done." << endl;

	  // unsubscribe from status and subband statistics
	  RSPUnsubstatusEvent unsubstatus;
	  unsubstatus.handle = m_statushandle;
	  if (!m_server.send(unsubstatus))
	    {
	      cout << "500 Error: failed to send subscription for status updates" << endl;
	      exit(EXIT_FAILURE);
	    }
	} else {
	  exit(EXIT_SUCCESS);
	}
      }
    }
    break;

    case RSP_UNSUBSTATUSACK:
    {
      RSPUnsubstatusackEvent ack(e);
      
      if (SUCCESS != ack.status)
      {
	cout << "500 Error: unsubscribe failure" << endl;
	exit(EXIT_FAILURE);
      }

      // unsubscribe from stats updates
      RSPUnsubstatsEvent unsubstats;
      unsubstats.handle = m_statshandle;
      if (!m_server.send(unsubstats))
	{
	  cout << "500 Error: failed to send subscription for status updates" << endl;
	  exit(EXIT_FAILURE);
	}
    }
    break;

    case RSP_UNSUBSTATSACK:
    {
      RSPUnsubstatsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
	cout << "500 Error: unsubscribe failure" << endl;
	exit(EXIT_FAILURE);
      }

      if (HTTP_MODE == m_options.xinetd_mode) {
	exit(EXIT_SUCCESS);
      }
      TRAN(RSPCtl::wait4command);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();
      cout << "500 Error: port '" << port.getName() << "' disconnected." << endl;
      exit(EXIT_FAILURE);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

bool RSPCtl::capture_statistics(Array<double, 2>& stats)
{
  if (0 == m_nseconds)
  {
    // initialize values array
    m_values.resize(stats.extent(firstDim), stats.extent(secondDim));
    m_values = 0.0;
  }
  else
  {
    if ( (stats.extent(firstDim) != m_values.extent(firstDim))
	 || (stats.extent(secondDim) != m_values.extent(secondDim)) )
    {
      cout << "500 Error: shape mismatch" << endl;
      exit(EXIT_FAILURE);
    }
  }

  m_values += stats; // integrate

  m_nseconds++; // advance to next second

  if (0 == m_nseconds % m_options.integration)
  {
    if (m_options.integration > 0) 
    {
      m_values /= m_options.integration;

      output_statistics(m_values); // write (optionally integrated) statistics

      if (!m_options.onefile && m_file)
      {
	for (int i = 0; i < m_options.n_devices; i++)
	{
	  if (m_file[i]) fclose(m_file[i]);
	  m_file[i] = 0;
	}
      }
    }
    else
    {
      output_statistics(stats); // write interval sampled statistics

      if (!m_options.onefile && m_file)
      {
	for (int i = 0; i < m_options.n_devices; i++)
	{
	  if (m_file[i]) fclose(m_file[i]);
	  m_file[i] = 0;
	}
      }
    }
    
    m_values = 0.0;
  }

  // check if duration has been reached
  if (m_nseconds >= m_options.duration)
  {
    if (m_file) {
      for (int i = 0; i < m_options.n_devices; i++)
	{
	  if (m_file[i]) fclose(m_file[i]);
	  m_file[i] = 0;
	}
    }

    return true;
  }

  return false;
}

void RSPCtl::output_statistics(Array<double, 2>& stats)
{
  int result_device = 0;

  for (int device = 0; device < m_options.n_devices; device++)
  {
    if (!m_options.device_set[device]) continue;
    
    time_t now = time(0);
    struct tm* t = localtime(&now);

    if (!t)
    {
      cout << "500 Error: localtime?" << endl;
      exit(EXIT_FAILURE);
    }

    if (m_file && !m_file[device] && (CMDLINE_MODE == m_options.xinetd_mode))
    {
      char filename[PATH_MAX];
      switch (m_options.type)
      {
	case Statistics::SUBBAND_POWER:
	  snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_sst_rcu%02d.dat",
		   t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		   t->tm_hour, t->tm_min, t->tm_sec, device);
	  break;
	case Statistics::BEAMLET_POWER:
	  snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_bst_pol%02d.dat",
		   t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		   t->tm_hour, t->tm_min, t->tm_sec, device);
	  break;
	default:
	  cout << "500 Error: invalid type" << endl;
	  exit(EXIT_FAILURE);
	  break;
      }
      m_file[device] = fopen(filename, "w");

      if (!m_file[device])
      {
	cout << "500 Error: Failed to open file: " << filename << endl;
	exit(EXIT_FAILURE);
      }
    }
    
    switch (m_options.xinetd_mode)
      {
      case CMDLINE_MODE:
	if (stats.extent(secondDim)
	    != (int)fwrite(stats(result_device, Range::all()).data(), sizeof(double),
			   stats.extent(secondDim), m_file[device]))
	{
	  cout << "500 Error: fwrite" << endl;
	  exit(EXIT_FAILURE);
	}
	break;

      case XINETD_MODE:
	cout << "250-META rcu " << device << endl;
	cout << "250-BEGIN" << endl;
	for (int i = 0; i < stats.extent(secondDim); i++) {
	  printf(m_format.c_str(), stats(result_device, i));
	}
	cout << "250-END" << endl;
	cout << "250-" << endl;
	break;

      case HTTP_MODE:
	for (int i = 0; i < stats.extent(secondDim); i++) {
	  printf("%f\n", stats(result_device, i));
	}
	break;
    }

    result_device++; // next
  }

}
#endif

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
  cout << "  Examples --select sets: --select=1,2,4:7 or --select=1:3,5:7" << endl;
  cout << "rspctl --weights=(0|1)  [--select=<set>] # set weights" << endl;
  cout << "rspctl --subbands       [--select=<set>] # get subband selection" << endl;
  cout << "rspctl --subbands=<set> [--select=<set>] # set subband selection" << endl;
  cout << "  Eexample --subbands sets: --subbands=0:39 or --select=0:19,40:59" << endl;
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
	      }
	    }
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
    usage();
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
