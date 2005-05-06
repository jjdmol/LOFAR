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

// local funtions
static std::bitset<MAX_N_RCUS> strtoset(const char* str, unsigned int max);
static void usage(const char* xinetdprefix = 0);
static int  parse_options(int argc, char** argv, CaptureStats::Options& options);

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
   
#define SPC_BASE16_TO_10(x) (((x) >= '0' && (x) <= '9') ? ((x) - '0') : \
                             (toupper((x)) - 'A' + 10))
   
char *spc_decode_url(const char *url, size_t *nbytes) {
  char       *out, *ptr;
  const char *c;
   
  if (!(out = ptr = strdup(url))) return 0;
  for (c = url;  *c;  c++) {
    if (*c != '%' || !isxdigit(c[1]) || !isxdigit(c[2])) *ptr++ = *c;
    else {
      *ptr++ = (SPC_BASE16_TO_10(c[1]) * 16) + (SPC_BASE16_TO_10(c[2]));
      c += 2;
    }
  }
  *ptr = 0;
  if (nbytes) *nbytes = (ptr - out); /* does not include null byte */
  return out;
}

CaptureStats::CaptureStats(string name, const Options& options)
  : GCFTask((State)&CaptureStats::initial, name),
    m_nseconds(0), m_file(0), m_format("250-%f\n")
{
  m_options = options;

  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  if (CMDLINE_MODE == m_options.xinetd_mode) {
    m_file = new (FILE*)[m_options.n_devices];
    if (!m_file)
      {
	cout << "500 Error: failed to allocate memory for file handles." << endl;
	exit(EXIT_FAILURE);
      }
    for (int i = 0; i < m_options.n_devices; i++) m_file[i] = 0;
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
      if (XINETD_MODE == m_options.xinetd_mode) {
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

static std::bitset<MAX_N_RCUS> strtoset(const char* str, unsigned int max)
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

void CaptureStats::parse_urloptions(char* url)
{
  while (url && *url && *url++ == ' ') { /*nop*/ } // skip whitespace

  // url+1 skips past leading slash
  char *key, *value = 0;

  char *p = url+1;
  while (p && *p) {

    key = p;
    while (*p && *p++ != '='); // point to value
    if (*p) {
      *(p-1) = 0; // terminate key

      value = p;
      while (*p && *p++ != '&'); // point to next key
      if (*p) *(p-1) = 0; // terminate value
    }
    
    char *decoded_value = spc_decode_url(value, 0);
    //cout << "value=\"" << decoded_value << "\"" << endl;

    string option = key;
    if ("device_set" == option) {
      m_options.device_set = strtoset(decoded_value, m_options.n_devices);
    } else if ("duration" == option) {
      m_options.duration = atoi(decoded_value);
    } else if ("integration" == option) {
      m_options.integration = atoi(decoded_value);
    } else if ("rcucontrol" == option) {
      unsigned long controlopt = strtoul(decoded_value, 0, 0);
      if ( controlopt > 0xFF )
	{
	  cout << "500 Error: invalid control parameter, must be < 0xFF" << endl;
	  return;
	}
      m_options.rcucontrol = (uint8)controlopt;
    } else if ("type" == option) {
      m_options.type = atoi(decoded_value);
    } else if ("submit" == option) {
      /* ignore */
    } else {
      cout << "500 Warning: unrecognised option: " << key << "=\"" << decoded_value << "\"" << endl;
      return;
    }
    
    if (decoded_value) {
      free(decoded_value);
      decoded_value = 0;
    }
    
    //cout << "key=\"" << key << "\"" << endl;
    //cout << "value=\"" << value << "\"" << endl;
  }
}

GCFEvent::TResult CaptureStats::wait4command(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	string symbol;
	//int    station_id;
	string rcuspec;
	bool done = false;
	char line[MAX_LINE_LENGTH];
	int   argc = 0;
	char* argv[20];

	while (!done && cin) {

	  // get command
	  cin >> symbol;

	  if ("GET" == symbol) {

	    cout << "HTTP/1.1 200 OK" << endl;
	    cout << "Content-Type: application/text" << endl;
	    cout << "Content-Disposition: attachment; filename=\"data.dat\"" << endl;
	    cout << endl;

	    m_options.xinetd_mode = HTTP_MODE; // HTTP mode

	    // read URL parameters
	    line[MAX_LINE_LENGTH-1] = '\0'; // make sure it is NULL-terminated
	    cin.getline(line, MAX_LINE_LENGTH - 1);

	    //cout << "line=\"" << line << "\"" << endl;
	    parse_urloptions(line);

#if 0	    
	    // read away remainder of HTTP request
	    do {
	      cin.getline(line, MAX_LINE_LENGTH - 1);
	    } while(strlen(line) > 0);
#endif
	    done=true;
	    TRAN(CaptureStats::handlecommand);

	  } else if ("hello" == symbol) {

	    // send welcome
	    cout << "220 This is the CaptureStats server, at your service." << endl;

	  } else if ("set" == symbol) {

	    // xinetd mode
	    cin >> symbol;

	    if ("sensor" == symbol) {

	      cin >> symbol;

	      if ("statistics" == symbol) {
		
		// get options
		memset(line, '\n', MAX_LINE_LENGTH); // initialize
		cin.getline(line, MAX_LINE_LENGTH - 1);
		memcpy(m_line, line, MAX_LINE_LENGTH);

		// convert to argc, argv
		char* c = line;
		argv[0] = "set sensor statistics";
		argc = 1;
		while (*c && *c != '\n' && argc < MAX_OPTIONS) {
		  if (*c == ' ') {
		    while (*c && *++c == ' ') { /*nop*/ } // skip whitespace
		    *(c-1) = 0;
		    argv[argc++] = c;
		  }
		  c++;
		}

		// parse options
		if (0 == parse_options(argc, argv, m_options)) {
		  cout << "210 Ok." << endl;
		}
		m_options.xinetd_mode = XINETD_MODE; // this was reset by parse_options

#if 0
		cin >> station_id;
		cin >> rcuspec;
		cin >> m_options.duration;
		cin >> m_options.integration;
		cin.ignore(256, '\n');

		m_options.device_set = strtoset(rcuspec.c_str(), m_options.n_devices);
		if (m_options.duration < 1) {

		  cout << "500 Bad set command" << endl;

		} else {

		  if (0 == m_options.integration) m_options.integration = m_options.duration;
		  cout << "210 Ok." << endl;

		}
#endif

	      } else {

		cout << "500 Syntax error" << endl;

	      }
	  
	    } else if ("format" == symbol) {

	      cin >> m_format;
	      m_format = "250-" + m_format + "\n"; // prepend 250- prompt
	      cout << "210 Ok." << endl;

	    } else {

	      cout << "500 Syntax error" << endl;

	    }
	  } else if ("go" == symbol) {

	    done = true;
	    cin.ignore(256,'\n');
	    TRAN(CaptureStats::handlecommand);

	  } else if ("help" == symbol) {

	    usage("250-");
	    cout << "250 done." << endl;

	  } else if ("quit" == symbol) {

	    cout << "221 Bye." << endl;
	    exit(EXIT_SUCCESS);

	  }else {
	    cout << "500 Syntax error" << endl;
	  }
	}

	if (!cin) { // lost connection with stdin
	  exit(EXIT_FAILURE);
	}
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

GCFEvent::TResult CaptureStats::handlecommand(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(CaptureStats::wait4command);
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

void CaptureStats::output_statistics(Array<double, 2>& stats)
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

void CaptureStats::run()
{
  start(); // make initial transition
  GCFTask::run();
}

static void usage(const char* xinetdprefix)
{
  const char* p = (xinetdprefix?xinetdprefix:"");

  if (!xinetdprefix) {
    cout << p << "Usage: CaptureStats [options]" << endl;
  } else {
    cout << p << "Usage: set sensor statistics [options]" << endl;
  }

  cout << p << "Available options:" << endl;
  cout << p << "    --rcu=<rcuset>           # or -r; default 1, : means all RCU's" << endl;
  cout << p << "        Example: -r=1,2,4:7 or --rcu=1:3,5:7" << endl;
  cout << p << endl;
  cout << p << "    --control=0xHH           # or -c; default 0xB9 (low band)" << endl;
  cout << p << "        bit[7] = VDDVCC_ENABLE" << endl;
  cout << p << "        bit[6] = VH_ENABLE" << endl;
  cout << p << "        bit[5] = VL_ENABLE" << endl;
  cout << p << "        bit[4] = FILSEL_1" << endl;
  cout << p << "        bit[3] = FILSEL_0" << endl;
  cout << p << "        bit[2] = BANDSEL" << endl;
  cout << p << "        bit[1] = HBA_ENABLE" << endl;
  cout << p << "        bit[0] = LBA_ENABLE" << endl;
  cout << p << "        Exmples LB=0xB9, HB_110_190=0xC6, HB_170_230=0xCE, HB_210_250=0xD6" << endl;
  cout << p << endl;
  cout << p << "    --duration=N             # or -d; default 1, number of seconds to capture" << endl;
  cout << p << endl;
  cout << p << "    --integration[=N]        # or -i; default equal to duration" << endl;
  cout << p << "        Note: integration < 0 means don't itegrate but sample with the specified interval." << endl;
  cout << p << endl;
  cout << p << "    --statstype=0|1          # or -s; default 0, 0 = subbands, 1 = beamlets" << endl;
  cout << p << endl;

  if (!xinetdprefix) {
    // only for command line version
    cout << p << "    --onefile                # or -o; default off, output one big file or file per interval." << endl;
    cout << p << endl;
    cout << p << "    --xinetd                 # or -x; enable xinetd mode, input from stdin, output on stdout." << endl;
    cout << p << endl;
    cout << p << "    --help                   # or -h; this help" << endl;
    cout << p << endl;
    cout << p << "Examples:" << endl;
    cout << p << "  CaptureStats -r=5 -d=20 -i=5" << endl;
    cout << p << "   # capture from RCU 5 for 20 seconds integrating every 5 seconds; results in four output files." << endl;
    cout << p << endl;
    cout << p << "  CaptureStats -r=: -d=10 -i" << endl;
    cout << p << "   # capture from all RCUs for 10 seconds integrating 10 seconds; results in one output file for each RCU." << endl;
    cout << p << endl;
    cout << p << "  CaptureStats -r=: -d=10 -i=-2" << endl;
    cout << p << "   # capture from all RCUs for 10 seconds only storing every second result in a separate file for each RCU." << endl;
  } else {
    cout << p << "Examples:" << endl;
    cout << p << "  set sensor statistics -r=5 -d=20 -i=5" << endl;
    cout << p << "   # capture from RCU 5 for 20 seconds integrating every 5 seconds; results in four output files." << endl;
    cout << p << endl;
    cout << p << "  set sensor statistics -r=: -d=10 -i" << endl;
    cout << p << "   # capture from all RCUs for 10 seconds integrating 10 seconds; results in one output file for each RCU." << endl;
    cout << p << endl;
    cout << p << "  set sensor statistics -r=: -d=10 -i=-2" << endl;
    cout << p << "   # capture from all RCUs for 10 seconds only storing every second result in a separate file for each RCU." << endl;
  }
}

static int parse_options(int argc, char** argv, CaptureStats::Options& options)
{
  // print arguments
  //for (int i = 0; i < argc; i++) cout << "argv[" << i << "]=\"" << argv[i] << "\"" << endl;

  // set default options;
  options.type = 0;
  options.device_set.set(0);  // default is rcu 0
  options.n_devices = 0;
  options.duration = 1;
  options.integration = 1;
  options.rcucontrol = 0xB9;
  options.onefile = false;
  options.xinetd_mode = CaptureStats::CMDLINE_MODE;

  options.n_devices = ((options.type <= Statistics::SUBBAND_POWER) ?
		       GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL :
		       GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL);

  optind = 0; // reset option parsing
  //opterr = 0; // no error reporting to stderr
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
	if (optarg) options.device_set = strtoset(optarg, options.n_devices);
	break;
	
      case 'c':
	{
	  if (optarg) {
	    unsigned long controlopt = strtoul(optarg, 0, 0);
	    if ( controlopt > 0xFF )
	    {
	      cout << "500 Error: invalid control parameter, must be < 0xFF" << endl;
	      return -1;
	    }
	    options.rcucontrol = (uint8)controlopt;
#if 0
	    cout << formatString("control=0x%02x", rcucontrol) << endl;
#endif
	  }
	}
	break;

      case 'd':
	if (optarg) {
	  if (1 != sscanf(optarg, "%d", &options.duration)) {
	    cout << "500 format error for duration option" << endl;
	    return -1;
	  }
	} else {
	  cout << "500 missing argument for duration option" << endl;
	  return -1;
	}
	break;

      case 'i':
	if (optarg) {
	  if (1 != sscanf(optarg, "%d", &options.integration)) {
	    cout << "500 format error for integration option" << endl;
	    return -1;
	  }
	} else {
	  cout << "500 missing argument for integration option" << endl;
	  return -1;
	}
	break;
	
      case 't':
	if (optarg) {
	  if (1 != sscanf(optarg, "%d", &options.type)) {
	    cout << "500 format error for type option" << endl;
	    return -1;
	  }

	  if (options.type < 0 || options.type >= Statistics::N_STAT_TYPES)
	  {
	    cout << "500 " << formatString("Error: invalid type of stat, should be >= 0 && < %d", Statistics::N_STAT_TYPES) << endl;
	    return -1;
	  }
	} else {
	  cout << "500 missing argument for type option" << endl;
	  return -1;
	}
	break;
	
      case 'o':
	options.onefile = true;
	break;

      case 'x':
	options.xinetd_mode = CaptureStats::XINETD_MODE;
	break;

      case 'h':
	usage();
	return -1;
	break;
	
      case '?':
	cout << "500 Error: error in option '" << char(optopt) << "'." << endl;
	return -1;
	break;

      default:
	printf ("?? getopt returned character code 0%o ??\n", c);
	break;
    }
  }

  // check for valid
  if (options.device_set.count() == 0 || options.device_set.count() > (unsigned int)options.n_devices)
  {
    cout << "500 " << formatString("Error: invalid device set or device set not specified") << endl;
    return -1;
  }

  // check for valid duration
  if (options.duration <= 0)
  {
    cout << "500 Error: duration must be > 0." << endl;
    return -1;
  }

  if (0 == options.integration) options.integration = options.duration;

  // check for valid integration
  if ( (abs(options.integration) > options.duration) )
  {
    cout << "500 Error: abs(integration) must be <= duration" << endl;
    return -1;
  }
  
  return 0;
}

int main(int argc, char** argv)
{
  CaptureStats::Options options;

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

  if (parse_options(argc, argv, options) < 0) {
    exit(EXIT_FAILURE);
  }
  CaptureStats c("CaptureStats", options);
  
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
