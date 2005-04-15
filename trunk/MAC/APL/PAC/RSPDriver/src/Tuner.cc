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
#include <math.h>

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
#define SCALE (1<<(16-2))

#define SAMPLE_FREQUENCY 163.84e6 // MHz
#define DECIMATION       1024

Tuner::Tuner(string name, bitset<MAX_N_RCUS> device_set, int n_devices,
	     uint8 rcucontrol, int centersubband, bool initialize)
  : GCFTask((State)&Tuner::initial, name),
    m_device_set(device_set), m_n_devices(n_devices),
    m_rcucontrol(rcucontrol), m_centersubband(centersubband), m_initialize(initialize)
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
      if (m_initialize) {
	TRAN(Tuner::initialize);
      } else {
	TRAN(Tuner::tunein);
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

BZ_DECLARE_FUNCTION_RET(convert2complex_int16_t, complex<int16_t>)

/**
 * Convert the weights to 16-bits signed integer.
 */
inline complex<int16_t> convert2complex_int16_t(complex<double> cd)
{
#ifdef W_TYPE_DOUBLE
  return complex<int16_t>((int16_t)(round(cd.real()*SCALE)),
			  (int16_t)(round(cd.imag()*SCALE)));
#else
  return complex<int16_t>((int16_t)(roundf(cd.real()*SCALE)),
			  (int16_t)(roundf(cd.imag()*SCALE)));
#endif
}

GCFEvent::TResult Tuner::initialize(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	//
	// switch on and configure the selected RCU's
	//
	RSPSetrcuEvent setrcu;
	setrcu.timestamp = Timestamp(0,0);
	setrcu.rcumask = m_device_set;
      
	setrcu.settings().resize(1);
	setrcu.settings()(0).value = m_rcucontrol;

	if (!m_server.send(setrcu))
	  {
	    LOG_FATAL("Error: failed to send RCU control");
	    exit(EXIT_FAILURE);
	  }
      }
      break;
    
    case RSP_SETRCUACK:
      {
	RSPSetrcuackEvent ack(e);

	if (SUCCESS != ack.status)
	  {
	    LOG_FATAL("Error: failed to set RCU control register");
	    exit(EXIT_FAILURE);
	  }

	//
	// Clear subband selection
	//
	RSPSetsubbandsEvent ss;
	ss.timestamp = Timestamp(0,0);

	ss.blpmask.reset();
	for (int i = 0;
	     i < GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i);
	     i++)
	  {
	    ss.blpmask.set(i); // all blps
	  }

	ss.subbands().resize(1, MEPHeader::N_BEAMLETS * 2);
	ss.subbands() = 0;

	if (!m_server.send(ss))
	  {
	    LOG_FATAL("Error: failed to send subband selection");
	    exit(EXIT_FAILURE);
	  }

      }
      break;

    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);
	if (SUCCESS != ack.status)
	  {
	    LOG_FATAL("Error: negative ack on subband selection");
	    exit(EXIT_FAILURE);
	  }

	//
	// Clear the beamformer weights
	//
	RSPSetweightsEvent sw;
	sw.timestamp = Timestamp(0,0);

	sw.blpmask.reset();
	for (int i = 0;
	     i < GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i);
	     i++)
	  {
	    sw.blpmask.set(i);
	  }

	sw.weights().resize(1, GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i),
			    MEPHeader::N_BEAMLETS, MEPHeader::N_POL);
	sw.weights() = complex<int16>(0,0);
	
	if (!m_server.send(sw))
	  {
	    LOG_FATAL("Error: failed to send beamformer weights");
	    exit(EXIT_FAILURE);
	  }
      }
      break;

    case RSP_SETWEIGHTSACK:
      {
	RSPSetweightsackEvent ack(e);
	if (SUCCESS != ack.status)
	  {
	    LOG_FATAL("Error: negative ack on beamformer weights");
	    exit(EXIT_FAILURE);
	  }

	TRAN(Tuner::tunein);
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
	//STOP_TEST();
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}
GCFEvent::TResult Tuner::tunein(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (e.signal)
    {
    case F_ENTRY:
      {
	//
	// Select the appropriate subbands
	//
	RSPSetsubbandsEvent ss;
	ss.timestamp = Timestamp(0,0);

	ss.blpmask.reset();
	for (int i = 0; i < GET_CONFIG("RS.N_BLPS", i); i++)
	  {
	    if (m_device_set[i * MEPHeader::N_POL]
		|| m_device_set[i * MEPHeader::N_POL + 1])
	    {
	      LOG_DEBUG_STR("selecting blp " << i);
	      ss.blpmask.set(i); // only selected blp
	    }
	  }

	ss.subbands().resize(1, MEPHeader::N_BEAMLETS * 2);
	ss.subbands() = 0;

	for (int i = 0; i < N_SELECTED_SUBBANDS; i++)
	  {
	    ss.subbands()(0, i*2)   = (m_centersubband + i - (N_SELECTED_SUBBANDS/2)) * 2;
	    ss.subbands()(0, i*2+1) = (m_centersubband + i - (N_SELECTED_SUBBANDS/2)) * 2 + 1;

	    LOG_DEBUG_STR("subband(" << i*2 << ")=" << (m_centersubband + i - (N_SELECTED_SUBBANDS/2)) * 2);
	    LOG_DEBUG_STR("subband(" << i*2+1 << ")=" << (m_centersubband + i - (N_SELECTED_SUBBANDS/2)) * 2 + 1);
	  }
      
	if (!m_server.send(ss))
	  {
	    LOG_FATAL("Error: failed to send subband selection");
	    exit(EXIT_FAILURE);
	  }

      }
      break;

    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);
	if (SUCCESS != ack.status)
	  {
	    LOG_FATAL("Error: negative ack on subband selection");
	    exit(EXIT_FAILURE);
	  }

	//
	// Set beamformer weights
	//
	RSPSetweightsEvent sw;
	sw.timestamp = Timestamp(0,0);

	sw.blpmask.reset();
	for (int i = 0; i < GET_CONFIG("RS.N_BLPS", i); i++)
	  {
	    if (m_device_set[i * MEPHeader::N_POL]
		|| m_device_set[i * MEPHeader::N_POL + 1])
	    {
	      LOG_DEBUG_STR("selecting blp " << i);
	      sw.blpmask.set(i); // only selected blp
	    }
	  }

	Array<complex<double>, 4> weights(1, GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i),
					  MEPHeader::N_BEAMLETS, MEPHeader::N_POL);

	weights(0, Range::all(), Range::all(), Range::all()) = complex<double>(0,0);
	for (int rcu = 0;
	     rcu < GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i) * MEPHeader::N_POL;
	     rcu++)
	  {
	    if (m_device_set[rcu]) {
	      weights(0, rcu/2, Range::all(), rcu%2) = complex<double>(1,0);
	    }
	  }

	sw.weights().resize(1, GET_CONFIG("RS.N_BLPS", i) * GET_CONFIG("RS.N_RSPBOARDS", i),
			    MEPHeader::N_BEAMLETS, MEPHeader::N_POL);
	sw.weights() = convert2complex_int16_t(conj(weights));
	
	if (!m_server.send(sw))
	  {
	    LOG_FATAL("Error: failed to send beamformer weights");
	    exit(EXIT_FAILURE);
	  }
      }
      break;

    case RSP_SETWEIGHTSACK:
      {
	RSPSetweightsackEvent ack(e);
	if (SUCCESS != ack.status)
	  {
	    LOG_FATAL("Error: negative ack on beamformer weights");
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
	//STOP_TEST();
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
  cout << "    --frequency=N            # or -f; default 88.0MHz, frequency of center subband" << endl;
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
      LOG_FATAL_STR("Value " << rcu << " out of range in RCU set specification");
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
	      LOG_FATAL("Error: invalid rcu range specified");
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

int freq2subband(double frequency)
{
  double subband = (frequency / SAMPLE_FREQUENCY) * DECIMATION;

  if (subband > 512) subband = 512 - (subband - 512); // correct for aliasing

  LOG_INFO_STR("freq2subband: subband=" << subband);

  return int(floor(subband));
}

int main(int argc, char** argv)
{
  bitset<MAX_N_RCUS> device_set = 0;
  unsigned long controlopt = 0xB9;
  uint8 rcucontrol = 0xB9;
  int centersubband = 0;
  double frequency = 88.0e6;
  bool initialize = false;
  
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
      LOG_FATAL_STR("Error: failed to load configuration files: " << e.text());
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
	  { "frequency",     required_argument, 0, 'f' },
	  { "init",          no_argument,       0, 'i' },
	  { "help",          no_argument,       0, 'h' },
	  { 0, 0, 0, 0 },
	};

      int option_index = 0;
      int c = getopt_long_only(argc, argv,
			       "r:c:s:f:ih", long_options, &option_index);
    
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
	      LOG_FATAL("Error: invalid control parameter, must be < 0xFF");
	      exit(EXIT_FAILURE);
	    }
	  rcucontrol = controlopt;
#if 0
	  LOG_DEBUG(formatString("control=0x%02x", rcucontrol));
#endif
	  break;
      
	case 's':
	  if (optarg) centersubband = atoi(optarg);
	  LOG_DEBUG_STR("centersubband=" << centersubband);
	  break;
	  
	case 'f':
	  if (optarg) frequency = atof(optarg) * 1e6;
	  LOG_DEBUG_STR("frequency=" << frequency);
	  break;
      
	case 'h':
	  usage();
	  exit(EXIT_SUCCESS);
	  break;

	case 'i':
	  initialize = true;
	  break;
      
	case '?':
	  LOG_FATAL_STR("Error: error in option '" << char(optopt) << "'.");
	  exit(EXIT_FAILURE);
	  break;
      
	default:
	  printf ("?? getopt returned character code 0%o ??\n", c);
	  break;
	}
    }

  // convert frequency to center subband
  if (0 == centersubband)
  {
    centersubband = freq2subband(frequency);
  }

  // check for valid
  if (device_set.count() == 0 || device_set.count() > (unsigned int)n_devices)
    {
      LOG_FATAL("Error: invalid device set or device set not specified");
      exit(EXIT_FAILURE);
    }

  if (centersubband < N_SELECTED_SUBBANDS/2 || centersubband >= 512-(N_SELECTED_SUBBANDS/2))
    {
      LOG_WARN_STR("Warning: invalid center subband index, must be >= " << N_SELECTED_SUBBANDS/2 
		   << " and < " << 512-(N_SELECTED_SUBBANDS/2));
    }

  Tuner t("Tuner", device_set, n_devices, rcucontrol, centersubband, initialize);
  try
    {
      t.run();
    }
  catch (Exception e)
    {
      LOG_FATAL_STR("Error: exception: " << e.text());
      exit(EXIT_FAILURE);
    }

#if 0
  Suite s("Tuner", &cerr);
  s.addTest(new Tuner(
  try
    {
      s.run();
    }
  catch (Exception e)
    {
      LOG_FATAL_STR("Error: exception: " << e.text());
      exit(EXIT_FAILURE);
    }
  long nFail = s.report();
  s.free();
#endif

  LOG_INFO("Normal termination of program");

  return 0;
}
