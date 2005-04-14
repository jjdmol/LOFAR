//#
//#  Tuner.cc: implementation of Tuner class
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

#include "Tuner.h"

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

#define N_SELECTED_SUBBANDS 13

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

Tuner::Tuner(string name, bitset<MAX_N_RCUS> device_set, int n_devices,
	     uint8 rcucontrol, int centersubband)
  : GCFTask((State)&Tuner::initial, name), Test(name),
    m_device_set(device_set), m_n_devices(n_devices),
    m_rcucontrol(rcucontrol), m_centersubband(centersubband)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_server.init(*this, "server", GCFPortInterface::SAP, RSP_PROTOCOL);
}

Tuner::~Tuner()
{
}

GCFEvent::TResult Tuner::initial(GCFEvent& e, GCFPortInterface& port)
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
      TRAN(Tuner::enabled);
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

GCFEvent::TResult Tuner::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	START_TEST("enabled", "test UPDSTATS");

	//
	// switch on and configure the selected RCU's
	//
	RSPSetrcuEvent setrcu;
	setrcu.timestamp.setNow();
	setrcu.rcumask = m_device_set;
      
	setrcu.settings().resize(1);
	setrcu.settings()(0).value = m_rcucontrol;

	if (!m_server.send(setrcu))
	  {
	    cerr << "Error: failed to send RCU control" << endl;
	    exit(EXIT_FAILURE);
	  }
      }
      break;
    
    case RSP_SETRCUACK:
      {
	RSPSetrcuackEvent ack(e);

	if (SUCCESS != ack.status)
	  {
	    cerr << "Error: failed to set RCU control register" << endl;
	    exit(EXIT_FAILURE);
	  }

	sleep(2);

	//
	// Select the appropriate subbands
	//
	RSPSetsubbandsEvent ss;
	ss.timestamp.setNow(0);

	ss.blpmask.reset();
	for (int i = 0; i < GET_CONFIG("RS.N_BLPS", i); i++) ss.blpmask.set(i); // all blps

	ss.subbands().resize(1, MEPHeader::N_BEAMLETS * 2);
	ss.subbands() = 0;

	for (int i = 0; i < N_SELECTED_SUBBANDS; i++)
	  {
	    ss.subbands()(0, i*2)   = (m_centersubband + i - (N_SELECTED_SUBBANDS/2-1)) * 2;
	    ss.subbands()(0, i*2+1) = (m_centersubband + i - (N_SELECTED_SUBBANDS/2-1)) * 2 + 1;
	  }
      
	if (!m_server.send(ss))
	  {
	    cerr << "Error: failed to send subband selection" << endl;
	    exit(EXIT_FAILURE);
	  }

      }
      break;

    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);
	if (SUCCESS != ack.status)
	  {
	    cerr << "Error: negative ack on subband selection" << endl;
	    exit(EXIT_FAILURE);
	  }

	//
	// Set beamformer weights
	//
	RSPSetweightsEvent sw;
	sw.timestamp.setNow(0); // immediate

	sw.blpmask.reset();
	for (int i = 0; i < GET_CONFIG("RS.N_BLPS", i); i++) sw.blpmask.set(i); // all blps

	sw.weights().resize(1, GET_CONFIG("RS.N_BLPS", i), MEPHeader::N_BEAMLETS, MEPHeader::N_POL);
	sw.weights() = 0;
	
	if (!m_server.send(sw))
	  {
	    cerr << "Error: failed to send beamformer weights" << endl;
	    exit(EXIT_FAILURE);
	  }
      }
      break;

    case RSP_SETWEIGHTSACK:
      {
	RSPSetweightsackEvent ack(e);
	if (SUCCESS != ack.status)
	  {
	    cerr << "Error: negative ack on beamformer weights" << endl;
	    exit(EXIT_FAILURE);
	  }

	GCFTask::stop(); // stop the program
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();
	TRAN(Tuner::initial);
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

void Tuner::run()
{
  start(); // make initial transition
  GCFTask::run();
}

void usage()
{
  cout << "Usage: Tuner [arguments]" << endl;
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
  cout << "    --centersubband=N        # or -s; default 6, center subband (with 6 subbands to the left and 6 to the right" << endl;
  cout << endl;
  cout << "    --help                   # or -h; this help" << endl;
  cout << endl;
  cout << "Example:" << endl;
  cout << "  Tuner -r=5 -s=474" << endl;
  cout << "   # tune RCU5 to center subband 5" << endl;
  cout << endl;
}

std::bitset<MAX_N_RCUS> strtoset(char* str, unsigned int max)
{
  char* start = str;
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
      cerr << "Value " << rcu << " out of range in RCU set specification" << endl;
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
	      cerr << "Error: invalid rcu range specified" << endl;
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

int main(int argc, char** argv)
{
  bitset<MAX_N_RCUS> device_set = 0;
  unsigned long controlopt = 0xB9;
  uint8 rcucontrol = 0xB9;
  int centersubband = N_SELECTED_SUBBANDS - 1;
  
  // default is rcu 0
  device_set.set(0);
  
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

  int n_devices = GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL;

  while (1)
    {
      static struct option long_options[] = 
	{
	  { "rcu",           required_argument, 0, 'r' },
	  { "control",       required_argument, 0, 'c' },
	  { "centersubband", required_argument, 0, 's' },
	  { "help",          no_argument,       0, 'h' },
	  { 0, 0, 0, 0 },
	};

      int option_index = 0;
      int c = getopt_long_only(argc, argv,
			       "r:c:sh", long_options, &option_index);
    
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
	      cerr << "Error: invalid control parameter, must be < 0xFF" << endl;
	      exit(EXIT_FAILURE);
	    }
	  rcucontrol = controlopt;
#if 0
	  cout << formatString("control=0x%02x", rcucontrol) << endl;
#endif
	  break;
      
	case 's':
	  centersubband = atoi(optarg);
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

  // check for valid
  if (device_set.count() == 0 || device_set.count() > (unsigned int)n_devices)
    {
      cerr << "Error: invalid device set or device set not specified" << endl;
      exit(EXIT_FAILURE);
    }

  if (centersubband <= N_SELECTED_SUBBANDS/2-1 || centersubband >= 512-(N_SELECTED_SUBBANDS/2-1))
    {
      cerr << "Error: invalid center subband index, must be >= " << N_SELECTED_SUBBANDS/2-1 << " and < " << 512-(N_SELECTED_SUBBANDS/2-1) << endl;
    }

  Suite s("Tuner", &cerr);
  s.addTest(new Tuner("Tuner", device_set, n_devices, rcucontrol, centersubband));
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
