//#
//#  CaptureStats.cc: implementation of CaptureStats class
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

#include "CaptureStats.h"

#include "PSAccess.h"

#include <Suite/suite.h>
#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>
#include <getopt.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace LOFAR;
using namespace blitz;
using namespace EPA_Protocol;
using namespace RSP_Protocol;

#define SAMPLE_FREQUENCY 160.0 // MHz

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

CaptureStats::CaptureStats(string name, int type, int device, int n_devices, int duration, int integration)
  : GCFTask((State)&CaptureStats::initial, name), Test(name),
    m_type(type), m_device(device), m_n_devices(n_devices),
    m_duration(duration), m_integration(integration),
    m_nseconds(0)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

CaptureStats::~CaptureStats()
{}

GCFEvent::TResult CaptureStats::initial(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(CaptureStats::enabled);
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

GCFEvent::TResult CaptureStats::enabled(GCFEvent& e, GCFPortInterface& port)
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
      if (m_device >= 0)
      {
	substats.rcumask.set(m_device);
      }
      else
      {
	for (int i = 0; i < m_n_devices; i++)
	  substats.rcumask.set(i);
      }
	
      substats.period = 1;
      substats.type = m_type;
      substats.reduction = SUM;
      
      if (!m_server.send(substats))
      {
	cerr << "Error: failed to subscribe" << endl;
	exit(EXIT_FAILURE);
      }
    }
    break;

    case RSP_SUBSTATSACK:
    {
      RSPSubstatsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
	cerr << "Error: failed to subscribe" << endl;
	exit(EXIT_FAILURE);
      }
    }
    break;

    case RSP_UPDSTATS:
    {
      RSPUpdstatsEvent upd(e);

      if (SUCCESS != upd.status)
      {
	cerr << "Error: invalid update" << endl;
	exit(EXIT_FAILURE);
      }

#if 0
      LOG_INFO_STR("upd.time=" << upd.timestamp);
      LOG_INFO_STR("upd.handle=" << upd.handle);
      
      LOG_DEBUG_STR("upd.stats=" << upd.stats());
#endif

      capture_statistics(upd.stats());
    }
    break;
    
    case RSP_UNSUBSTATSACK:
    {
      RSPUnsubstatsackEvent ack(e);

      if (SUCCESS != ack.status)
      {
	cerr << "Error: unsubscribe failure" << endl;
	exit(EXIT_FAILURE);
      }

#if 0
      LOG_INFO_STR("ack.time=" << ack.timestamp);
      LOG_INFO_STR("ack.handle=" << ack.handle);
#endif

      port.close();
      TRAN(CaptureStats::initial);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();
      TRAN(CaptureStats::initial);
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

void CaptureStats::capture_statistics(Array<double, 2>& stats)
{
  cerr << ".";
  
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
      cerr << "Error: shape mismatch" << endl;
      exit(EXIT_FAILURE);
    }
  }

  m_values += stats; // integrate

  m_nseconds++; // advance to next second

  if (0 == m_nseconds % m_integration)
  {
    m_values /= m_integration;
    write_statistics(m_values);
    m_values = 0.0;
  }

  // check if duration has been reached
  if (m_nseconds >= m_duration)
  {
    cerr << endl;
    exit(EXIT_SUCCESS);
  }
}

void CaptureStats::write_statistics(Array<double, 2>& stats)
{
  for (int device = stats.lbound(firstDim);
       device <= stats.ubound(firstDim);
       device++)
  {
    time_t now = time(0);
    struct tm* t = localtime(&now);

    if (!t)
    {
      cerr << "Error: localtime?" << endl;
      exit(EXIT_FAILURE);
    }

    char filename[PATH_MAX];
    switch (m_type)
    {
      case Statistics::SUBBAND_POWER:
	snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_sst_rcu%02d.dat",
		 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		 t->tm_hour, t->tm_min, t->tm_sec,
		 (m_device < 0 ? device : m_device));
	break;
      case Statistics::BEAMLET_POWER:
	snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_bst_pol%02d.dat",
		 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		 t->tm_hour, t->tm_min, t->tm_sec,
		 (m_device < 0 ? device : m_device));
	break;
      default:
	cerr << "Error: invalid m_type" << endl;
	exit(EXIT_FAILURE);
	break;
    }

    FILE* ofile = fopen(filename, "w");

    if (!ofile)
    {
      cerr << "Error: Failed to open file: " << filename << endl;
      exit(EXIT_FAILURE);
    }
    
    if (stats.extent(secondDim)
	!= (int)fwrite(stats(device, Range::all()).data(), sizeof(double),
		       stats.extent(secondDim), ofile))
    {
      perror("fwrite");
      exit(EXIT_FAILURE);
    }
    (void)fclose(ofile);
  }
}

void CaptureStats::run()
{
  start(); // make initial transition
  GCFTask::run();
}

void usage()
{
  cout << "Usage: CaptureStats [arguments]" << endl;
  cout << "arguments (all optional):" << endl;
  cout << "    --rcu=0..N_AVAIABLE_RCUS # or -r; default 0, -1 means ALL RCUs" << endl;
  cout << "    --duration=N             # or -d; default 1, number of seconds to capture" << endl;
  cout << "    --integration[=N]        # or -i; default equal to duration" << endl;
  cout << "        Note: integration must be a divisor of duration" << endl;
  cout << "    --statstype=0|1          # or -s; default 0, 0 = subbands, 1 = beamlets" << endl;
  cout << "    --help                   # or -h; this help" << endl;
  cout << endl;
  cout << "Example:" << endl;
  cout << "  CaptureStats -r=5 -d=20 -i=5" << endl;
  cout << "   # capture from RCU 5 for 20 seconds integrating every 5 seconds; results in four output files." << endl;
  cout << endl;
  cout << "  CaptureStats -r=-1 -d=10 -i" << endl;
  cout << "   # capture from all RCUs for 10 seconds integrating 10 seconds; results in one output file for each RCU." << endl;
}

int main(int argc, char** argv)
{
  int type        = 0;
  int device      = 0;
  int duration    = 1;
  int integration = -1;

  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  try
  {
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
  }
  catch (Exception e)
  {
    cerr << "Error: failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  int n_devices = ((type <= Statistics::SUBBAND_POWER) ?
		   GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL :
		   GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL);
  while (1)
  {
    static struct option long_options[] = 
      {
        { "rcu",          required_argument, 0, 'r' },
	{ "duration",     required_argument, 0, 'd' },
	{ "integration",  optional_argument, 0, 'i' },
	{ "statstype",    required_argument, 0, 's' },
	{ "help",         no_argument,       0, 'h' },
	{ 0, 0, 0, 0 },
      };

    int option_index = 0;
    int c = getopt_long_only(argc, argv,
			     "r:d:i::s:h", long_options, &option_index);
    
    if (c == -1) break;
    
    switch (c)
    {
      case 'r':
	device = atoi(optarg);
	if (device < -1 || device >= n_devices)
	{
	  cerr << formatString("Error: invalid device index, should be >= -1 && < %d; -1 indicates all devices", n_devices) << endl;
	  exit(EXIT_FAILURE);
	}
	break;

      case 'd':
	duration = atoi(optarg);
	break;

      case 'i':
	if (optarg) integration = atoi(optarg);
	break;
	
      case 't':
	type = atoi(optarg);

	if (type < 0 || type >= Statistics::N_STAT_TYPES)
	{
	  cerr << formatString("Error: invalid type of stat, should be >= 0 && < %d", Statistics::N_STAT_TYPES) << endl;
	  exit(EXIT_FAILURE);
	}
	break;
	
      case 'h':
	usage();
	exit(EXIT_SUCCESS);
	break;

      case '?':
	cerr << "Error: error in option '" << char(optopt) << "'." << endl;
	exit(EXIT_FAILURE);
	break;

      default:
	printf ("?? getopt returned character code 0%o ??\n", c);
	break;
    }
  }

  // check for valid duration
  if (duration <= 0)
  {
    cerr << "Error: duration must be > 0." << endl;
    exit(EXIT_FAILURE);
  }

  if (integration <= 0) integration = duration;

  // check for valid integration
  if ( (integration > duration) || (0 != duration % integration) )
  {
    cerr << "Error: integration must be > 0 and <= duration and a divisor of duration" << endl;
    exit(EXIT_FAILURE);
  }

  Suite s("CaptureStats", &cerr);
  s.addTest(new CaptureStats("CaptureStats", type, device, n_devices, duration, integration));
  try
  {
    s.run();
  }
  catch (Exception e)
  {
    cerr << "Error: exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
