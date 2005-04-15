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

CaptureStats::CaptureStats(string name, int type, bitset<MAX_N_RCUS> device_set, int n_devices,
			   int duration, int integration, uint8 rcucontrol, bool onefile, bool xinetd_mode)
  : GCFTask((State)&CaptureStats::initial, name),
    m_type(type), m_device_set(device_set), m_n_devices(n_devices),
    m_duration(duration), m_integration(integration), m_rcucontrol(rcucontrol),
    m_nseconds(0), m_file(0), m_onefile(onefile), m_xinetd_mode(xinetd_mode), m_format("250-%f\n")
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  if (!m_xinetd_mode) {
    m_file = new (FILE*)[m_n_devices];
    if (!m_file)
      {
	cout << "500 Error: failed to allocate memory for file handles." << endl;
	exit(EXIT_FAILURE);
      }
    for (int i = 0; i < m_n_devices; i++) m_file[i] = 0;
  }
  
  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

CaptureStats::~CaptureStats()
{
  if (m_file) delete [] m_file;
}

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
      if (m_xinetd_mode) {
	// send welcome
	cout << "220 This is the CaptureStats server, at your service." << endl;
	TRAN(CaptureStats::wait4command);
      } else {
	TRAN(CaptureStats::handlecommand);
      }
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

std::bitset<MAX_N_RCUS> strtoset(const char* str, unsigned int max)
{
  string inputstring(str);
  char* start = (char*)inputstring.c_str();
  char* end   = 0;
  bool  range = false;
  unsigned long prevrcu = 0;
  bitset<MAX_N_RCUS> rcuset;

  rcuset.reset();

  while (start)
  {
    unsigned long rcu = strtoul(start, &end, 10); // read decimal numbers
    start = (end ? (*end ? end + 1 : 0) : 0); // advance
    if (rcu >= max)
    {
      cout << "500 Value " << rcu << " out of range in RCU set specification" << endl;
      exit(EXIT_FAILURE);
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
	    if (0 == prevrcu && 0 == rcu) rcu = max - 1;
	    if (rcu < prevrcu)
	    {
	      cout << "500 Error: invalid rcu range specified" << endl;
	      exit(EXIT_FAILURE);
	    }
	    for (unsigned long i = prevrcu; i <= rcu; i++) rcuset.set(i);
	  }
	    
	  else
	  {
	    rcuset.set(rcu);
	  }
	  range=false;
	}
	break;

	case ':':
	  range=true;
	  break;

	default:
	  printf("invalid character %c", *end);
	  break;
      }
    }
    prevrcu = rcu;
  }

  return rcuset;
}

GCFEvent::TResult CaptureStats::wait4command(GCFEvent& e, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	string symbol;
	int    station_id;
	string rcuspec;
	bool done = false;

	while (!done) {
	  // get command
	  cin >> symbol;

	  if ("set" == symbol) {

	    cin >> symbol;

	    if ("sensor" == symbol) {

	      cin >> symbol;

	      if ("subband_statistics" == symbol) {

		cin >> station_id;
		cin >> rcuspec;
		cin >> m_duration;
		cin >> m_integration;
		cin.ignore(256, '\n');

		m_device_set = strtoset(rcuspec.c_str(), m_n_devices);
		if (m_duration < 1) {

		  cout << "500 Bad set command" << endl;

		}
		if (0 == m_integration) m_integration = m_duration;

		cout << "210 Ok." << endl;

	      } else {

		cout << "500 Syntax error" << endl;

	      }
	  
	    } else if ("format" == symbol) {

	      cin >> m_format;
	      m_format = "250-" + m_format + "\n"; // prepend 250- prompt

	    } else {

	      cout << "500 Syntax error" << endl;

	    }
	  } else if ("go" == symbol) {

	    done = true;
	    cin.ignore(256,'\n');
	    TRAN(CaptureStats::handlecommand);

	  } else if ("quit" == symbol) {
	    cout << "221 Bye." << endl;
	    exit(EXIT_SUCCESS);
	  }else {
	    cout << "500 Syntax error" << endl;
	  }
	}
      }
      break;
    }

  return status;
}

GCFEvent::TResult CaptureStats::handlecommand(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
  {
    case F_ENTRY:
    {
      if (m_xinetd_mode) {
	cout << "250-Acquiring sensor data." << endl;
	cout << "250-" << endl;
	cout << "250-META sensor = subband_statistics 0 ";
	bool comma = false;
	for (int i = 0; i < (int)m_device_set.size(); i++) {
	  if (m_device_set[i]) {
	    if (comma) {
	      cout << ","; comma = false;
	    }
	    cout << i; comma = true;
	  }
	}
	cout << " " << m_duration << " " << m_integration << endl;
	cout << "250-" << endl;
      }

      // set rcu control register
      RSPSetrcuEvent setrcu;
      setrcu.timestamp = Timestamp(0,0);
      setrcu.rcumask = m_device_set;
      
      setrcu.settings().resize(1);
      setrcu.settings()(0).value = m_rcucontrol;

      if (!m_server.send(setrcu))
      {
	cout << "500 Error: failed to send RCU control" << endl;
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

      substatus.rcumask = m_device_set;
      
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

      substats.rcumask = m_device_set;
      
      substats.period = 1;
      substats.type = m_type;
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

      if (!m_xinetd_mode) {
	cout << "time=" << upd.timestamp;
      } else {
	cout << "250-META time status = " << upd.timestamp << endl;
      }
      int result = 0;
      for (int device = 0; device < m_n_devices; device++)
      {
	if (!m_device_set[device]) continue;
	
	if (m_xinetd_mode) {
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
    break;

    case RSP_UPDSTATS:
    {
      RSPUpdstatsEvent upd(e);

      if (SUCCESS != upd.status)
      {
	cout << "500 Error: invalid update" << endl;
	exit(EXIT_FAILURE);
      }

      if (!m_xinetd_mode) {
	cout << "time=" << upd.timestamp;
      } else {
	cout << "250-META time subband_statistics = " << upd.timestamp << endl;
      }

      if (capture_statistics(upd.stats())) {
	if (m_xinetd_mode) {
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

      TRAN(CaptureStats::wait4command);
    }
    break;

    case F_DISCONNECTED:
    {
      port.close();
      TRAN(CaptureStats::initial);
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

bool CaptureStats::capture_statistics(Array<double, 2>& stats)
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

  if (0 == m_nseconds % m_integration)
  {
    if (m_integration > 0) 
    {
      m_values /= m_integration;

      output_statistics(m_values); // write (optionally integrated) statistics

      if (!m_onefile && m_file)
      {
	for (int i = 0; i < m_n_devices; i++)
	{
	  if (m_file[i]) fclose(m_file[i]);
	  m_file[i] = 0;
	}
      }
    }
    else
    {
      output_statistics(stats); // write interval sampled statistics

      if (!m_onefile && m_file)
      {
	for (int i = 0; i < m_n_devices; i++)
	{
	  if (m_file[i]) fclose(m_file[i]);
	  m_file[i] = 0;
	}
      }
    }
    
    m_values = 0.0;
  }

  // check if duration has been reached
  if (m_nseconds >= m_duration)
  {
    if (m_file) {
      for (int i = 0; i < m_n_devices; i++)
	{
	  if (m_file[i]) fclose(m_file[i]);
	  m_file[i] = 0;
	}
    }

    return true;
  }

  return false;
}

void CaptureStats::output_statistics(Array<double, 2>& stats)
{
  int result_device = 0;

  for (int device = 0; device < m_n_devices; device++)
  {
    if (!m_device_set[device]) continue;
    
    time_t now = time(0);
    struct tm* t = localtime(&now);

    if (!t)
    {
      cout << "500 Error: localtime?" << endl;
      exit(EXIT_FAILURE);
    }

    if (m_file && !m_file[device] && !m_xinetd_mode)
    {
      char filename[PATH_MAX];
      switch (m_type)
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
	  cout << "500 Error: invalid m_type" << endl;
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
    
    if (!m_xinetd_mode) {
      if (stats.extent(secondDim)
	  != (int)fwrite(stats(result_device, Range::all()).data(), sizeof(double),
			 stats.extent(secondDim), m_file[device]))
	{
	  cout << "500 Error: fwrite" << endl;
	  exit(EXIT_FAILURE);
	}
    } else {

      cout << "250-META rcu " << device << endl;
      cout << "250-BEGIN" << endl;
      for (int i = 0; i < stats.extent(secondDim); i++) {
	printf(m_format.c_str(), stats(result_device, i));
      }
      cout << "250-END" << endl;
      cout << "250-" << endl;
    }

    result_device++; // next
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
  cout << "    --rcu=<rcuset>           # or -r; default 1, : means all RCU's" << endl;
  cout << "        Example: -r=1,2,4:7 or --rcu=1:3,5:7" << endl;
  cout << endl;
  cout << "    --control=0xHH           # or -c; default 0xB9 (low band)" << endl;
  cout << "        bit[7] = VDDVCC_ENABLE" << endl;
  cout << "        bit[6] = VH_ENABLE" << endl;
  cout << "        bit[5] = VL_ENABLE" << endl;
  cout << "        bit[4] = FILSEL_1" << endl;
  cout << "        bit[3] = FILSEL_0" << endl;
  cout << "        bit[2] = BANDSEL" << endl;
  cout << "        bit[1] = HBA_ENABLE" << endl;
  cout << "        bit[0] = LBA_ENABLE" << endl;
  cout << "        Exmples LB=0xB9, HB_110_190=0xC6, HB_170_230=0xCE, HB_210_250=0xD6" << endl;
  cout << endl;
  cout << "    --duration=N             # or -d; default 1, number of seconds to capture" << endl;
  cout << endl;
  cout << "    --integration[=N]        # or -i; default equal to duration" << endl;
  cout << "        Note: integration < 0 means don't itegrate but sample with the specified interval." << endl;
  cout << endl;
  cout << "    --statstype=0|1          # or -s; default 0, 0 = subbands, 1 = beamlets" << endl;
  cout << endl;
  cout << "    --onefile                # or -o; default off, output one big file or file per interval." << endl;
  cout << endl;
  cout << "    --xinetd                 # or -x; enable xinetd mode, input from stdin, output on stdout." << endl;
  cout << endl;
  cout << "    --help                   # or -h; this help" << endl;
  cout << endl;
  cout << "Example:" << endl;
  cout << "  CaptureStats -r=5 -d=20 -i=5" << endl;
  cout << "   # capture from RCU 5 for 20 seconds integrating every 5 seconds; results in four output files." << endl;
  cout << endl;
  cout << "  CaptureStats -r=-1 -d=10 -i" << endl;
  cout << "   # capture from all RCUs for 10 seconds integrating 10 seconds; results in one output file for each RCU." << endl;
  cout << endl;
  cout << "  CaptureStats -r=-1 -d=10 -i=-2" << endl;
  cout << "   # capture from all RCUs for 10 seconds only storing every second result in a separate file for each RCU." << endl;
}

int main(int argc, char** argv)
{
  int type        = 0;
  bitset<MAX_N_RCUS> device_set = 0;
  int duration    = 1;
  int integration = 0;
  unsigned long controlopt = 0xB9;
  uint8 rcucontrol = 0xB9;
  bool onefile = false;
  bool xinetd_mode = false;
  
  // default is rcu 0
  device_set.set(0);
  
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

  int n_devices = ((type <= Statistics::SUBBAND_POWER) ?
		   GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL :
		   GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL);

  while (1)
  {
    static struct option long_options[] = 
      {
        { "rcu",          required_argument, 0, 'r' },
        { "control",      required_argument, 0, 'c' },
	{ "duration",     required_argument, 0, 'd' },
	{ "integration",  optional_argument, 0, 'i' },
	{ "statstype",    required_argument, 0, 's' },
	{ "onefile",      no_argument,       0, 'o' },
	{ "xinetd",       no_argument,       0, 'x' },
	{ "help",         no_argument,       0, 'h' },
	  
	{ 0, 0, 0, 0 },
      };

    int option_index = 0;
    int c = getopt_long_only(argc, argv,
			     "r:d:i::s:ho", long_options, &option_index);
    
    if (c == -1) break;
    
    switch (c)
    {
      case 'r':
	device_set = strtoset(optarg, n_devices);
	break;
	
      case 'c':
	controlopt = strtoul(optarg, 0, 0);
	if ( controlopt > 0xFF )
	{
	  cout << "500 Error: invalid control parameter, must be < 0xFF" << endl;
	  exit(EXIT_FAILURE);
	}
	rcucontrol = controlopt;
#if 0
	cout << formatString("control=0x%02x", rcucontrol) << endl;
#endif
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
	  cout << "500 " << formatString("Error: invalid type of stat, should be >= 0 && < %d", Statistics::N_STAT_TYPES) << endl;
	  exit(EXIT_FAILURE);
	}
	break;
	
      case 'o':
	onefile = true;
	break;

      case 'x':
	xinetd_mode = true;
	break;

      case 'h':
	usage();
	exit(EXIT_SUCCESS);
	break;
	
      case '?':
	cout << "500 Error: error in option '" << char(optopt) << "'." << endl;
	exit(EXIT_FAILURE);
	break;

      default:
	printf ("?? getopt returned character code 0%o ??\n", c);
	break;
    }
  }

  // check for valid
  if (device_set.count() == 0 || device_set.count() > (unsigned int)n_devices)
  {
    cout << "500 " << formatString("Error: invalid device set or device set not specified") << endl;
    exit(EXIT_FAILURE);
  }

  // check for valid duration
  if (duration <= 0)
  {
    cout << "500 Error: duration must be > 0." << endl;
    exit(EXIT_FAILURE);
  }

  if (0 == integration) integration = duration;

  // check for valid integration
  if ( (abs(integration) > duration) )
  {
    cout << "500 Error: abs(integration) must be <= duration" << endl;
    exit(EXIT_FAILURE);
  }

  CaptureStats c("CaptureStats", type, device_set, n_devices, duration, integration, rcucontrol, onefile, xinetd_mode);
  
  try
  {
    c.run();
  }
  catch (Exception e)
  {
    cout << "500 Error: exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
