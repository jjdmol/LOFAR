//#
//#  beamctl.cc: implementation of beamctl class
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
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/BS_Protocol/BS_Protocol.ph>
#include <APL/RSP_Protocol/RCUSettings.h>
#include <GCF/GCF_ServiceInfo.h>

#include "beamctl.h"

#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <blitz/array.h>
#include <getopt.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

#define BEAMCTL_ARRAY "beamctl_array"
#define BEAMCTL_BEAM  "beamctl_beam"

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace RTC;
using namespace CAL_Protocol;
using namespace BS_Protocol;

beamctl::beamctl(string name,
		 string parent,
		 const list<int>& rcus,
		 RSP_Protocol::RCUSettings& rcumode,
		 const BS_Protocol::Pointing& direction)
  : GCFTask((State)&beamctl::initial, name), m_beamhandle(0),
    m_parent(parent), m_rcus(rcus), m_rcumode(rcumode), m_direction(direction)
{
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);
  registerProtocol(BS_PROTOCOL, BS_PROTOCOL_signalnames);

  m_calserver.init(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::SAP, CAL_PROTOCOL);
  m_beamserver.init(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::SAP, BS_PROTOCOL);
}

beamctl::~beamctl()
{}

GCFEvent::TResult beamctl::initial(GCFEvent& e, GCFPortInterface& port)
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
      m_calserver.open();
      m_beamserver.open();
    }
    break;

    case F_CONNECTED:
    {
      // if both calserver and beamserver are connected
      // then transition to connected state
      if (m_calserver.isConnected()
	  && m_beamserver.isConnected()) {
	TRAN(beamctl::create_subarray);
      }
    }
    break;

    case F_DISCONNECTED:
    {
      // retry once every second
      port.setTimer((long)1);
      port.close();
    }
    break;

    case F_TIMER:
    {
      // try again
      port.open();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult beamctl::create_subarray(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //
  // Create a new subarray
  //

  switch (e.signal)
    {
    case F_ENTRY:
      {
	CALStartEvent start;

	start.name   = BEAMCTL_ARRAY;
	start.parent = m_parent;
	start.subset = getRCUMask();
	start.nyquist_zone = m_rcumode()(0).getNyquistZone();
	start.rcumode = m_rcumode;
      }
      break;

    case CAL_STARTACK:
      {
	CALStartackEvent ack(e);

	if (ack.name != BEAMCTL_ARRAY || ack.status != CAL_Protocol::SUCCESS) {

	  cerr << "Error: fail to start calibration" << endl;
	  TRAN(beamctl::final);

	} else {

	  TRAN(beamctl::create_beam);

	}
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();

	cerr << "Error: unexpected disconnect" << endl;
	TRAN(beamctl::final);
      }
      break;

    case F_EXIT:
      {
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult beamctl::create_beam(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //
  // Create a new subarray
  //

  switch (e.signal)
    {
    case F_ENTRY:
      {
	BSBeamallocEvent alloc;
	
	alloc.name = BEAMCTL_BEAM;
	alloc.subarrayname = BEAMCTL_ARRAY;

	for (int i = 0; i < 10; i++)
	{
	    alloc.allocation()[i] = i;
	}

	m_beamserver.send(alloc);
      }
      break;

      case BS_BEAMALLOCACK:
      {
	BSBeamallocackEvent ack(e);

	if (BS_Protocol::SUCCESS != ack.status) {
	  cerr << "Error: failed to allocate beam" << endl;
	  TRAN(beamctl::final);
	} else {

	  m_beamhandle = ack.handle;
	  LOG_DEBUG(formatString("got beam_handle=%d", m_beamhandle));

	}

	// send pointto command (zenith)
	BSBeampointtoEvent pointto;
	pointto.handle = ack.handle;

	pointto.pointing = m_direction;

	m_beamserver.send(pointto);
      }
      break;

    case F_DISCONNECTED:
      {
	port.close();

	cerr << "Error: unexpected disconnect" << endl;
	TRAN(beamctl::final);
      }
      break;

    case F_EXIT:
      {
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult beamctl::final(GCFEvent& e, GCFPortInterface& /*port*/)
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

void usage()
{
  cout <<
"Usage: beamctl\n"
"  --array=name   # name of  array of which the RCU's are part (for positions)\n"
"  [--rcus=<set>] # select set of RCU's, all RCU's if not specified\n"
"  --rcumode=0..7 # RCU mode to use\n"
"  --direction=lon,lat[,type] # lon, lat are double values\n"
"                             # type is one of J2000 (default), AZEL, LOFAR_LMN,\n"
"  --help         # print this usage\n"
"\n"
"This utility connects to the CalServer to create a subarray of --array\n"
"containing the selected RCU's. The CalServer sets those RCU's in the mode\n"
"specified by --rcumode. Another connection is made to the BeamServer to create a\n"
"beam on the created subarray pointing in the direction specified with --direction.\n"
  << endl;
}

std::bitset<MAX_N_RCUS> beamctl::getRCUMask() const
{
  std::bitset<MAX_N_RCUS> mask;
  
  mask.reset();
  std::list<int>::const_iterator it;
  int count = 0; // limit to ndevices
  for (it = m_rcus.begin(); it != m_rcus.end(); ++it, ++count) {
    if (count >= MAX_N_RCUS) break;
    
    if (*it < MAX_N_RCUS)
      mask.set(*it);
  }
  return mask;
}

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
      cerr << formatString("Error: value %ld out of range",val) << endl;
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
	  cerr << formatString("Error: invalid character %c",*end) << endl;
          resultset.clear();
          return resultset;
          break;
      }
    }
    prevval = val;
  }

  return resultset;
}

void beamctl::mainloop()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  LOG_INFO(formatString("Program %s has started", argv[0]));

  char *array = "unset";              // --array argument goes here
  list<int> rcus;                     // --rcus argument goes here, default is to select all
  RSP_Protocol::RCUSettings rcumode;  // --rcumode argument goes here
  Pointing direction;                 // --direction argument goed here
  unsigned char presence = 0;         // keep track of which arguments have been specified

  // initialize rcus
  rcus.clear();
  for (int i = 0; i < MAX_N_RCUS; i++) rcus.push_back(i);

  // initialize rcumode
  rcumode().resize(1);

  // parse options
  optind = 0; // reset option parsing
  while (1) {

    static struct option long_options[] = {
      { "array",     required_argument, 0, 'a' },
      { "rcus",      optional_argument, 0, 'r' },
      { "rcumode",   required_argument, 0, 'm' },
      { "direction", required_argument, 0, 'd' },
      { "help",      no_argument,       0, 'h' },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "a:rm:d:h", long_options, &option_index);

    if (c == -1) break;

    switch (c) {

    case 'a':
      {
	if (!optarg) { cerr << "Error: missing --array argument" << endl; usage(); exit(EXIT_FAILURE); }
	array=strdup(optarg);
	presence |= 0x01;
      }
      break;
     
    case 'r':
      {
	if (!optarg) { cerr << "Error: missing --rcus argument" << endl; usage(); exit(EXIT_FAILURE); }
	rcus = strtolist(optarg, MAX_N_RCUS);
	// optional argument
      }
      break;

    case 'm':
      {
	if (!optarg) { cerr << "Error: missing --rcumode argument" << endl; usage(); exit(EXIT_FAILURE); }
	int mode = 0;
	if ((mode = atoi(optarg) < 0) || mode > 7) {
	  cerr << formatString("Error: --rcumode=%d, out of range [0..7]", mode) << endl;
	  exit(EXIT_FAILURE);
	}
	rcumode()(0).setMode((RSP_Protocol::RCUSettings::Control::RCUMode)mode);
	presence |= 0x02;
      }
      break;

    case 'd':
      {
	if (!optarg) { cerr << "Error: missing --direction argument" << endl; usage(); exit(EXIT_FAILURE); }
	presence |= 0x04;
      }
      break;

    case 'h':
      {
	usage();
	exit(EXIT_SUCCESS);
      }
      break;

    case '?':
    default:
      {
	//cerr << formatString("Error: unknown argument '%s'", optarg) << endl;
	exit(EXIT_FAILURE);
      }
      break;
    }
  }

  if (presence != 0x07) {
    cerr << "Error: Not all required arguments have been specified" << endl;
    usage();
    exit(EXIT_FAILURE);
  }

  GCFTask::init(argc, argv);

  beamctl ctl("beamctl", array, rcus, rcumode, direction);

  try {
    ctl.mainloop();
  } catch (Exception e) {
    cerr << "Exception: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  LOG_INFO("Normal termination of program");

  return 0;
}
