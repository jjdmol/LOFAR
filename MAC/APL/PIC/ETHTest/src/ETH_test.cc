#define DEBUG_SIGNAL

// 
//  ETH_test.cc: example code for explicit GCFETHRawPort
//
//  Copyright (C) 2003
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$


#include "GCF/TM/GCF_Control.h"
#include "GCF/TM/GCF_ETHRawPort.h"
#include "MEPData.h"
#include "DataEvent.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define MAX_IFNAME_LEN 64
#define HWADDR_LEN 17

using namespace LOFAR;

class ETH_test : public GCFTask
{
 public:

  ETH_test(const char* name, bool isClient,
	   const char* ifname, const char* dest_mac = "");

 private:

  int initial_state  (GCFEvent& e, GCFPortInterface& port);

 private:

  GCFETHRawPort channel;
};


ETH_test::ETH_test(const char* name, bool isClient,
		   const char* ifname, const char* dest_mac) :
    GCFTask((State)&ETH_test::initial_state, name), channel()
{
  // client or server?
  channel.init(*this, "channel", (isClient ? GCFPortInterface::SAP : GCFPortInterface::SPP), 0, true);
  channel.setAddr(ifname, dest_mac);
}

static char ETH_packet[ETH_DATA_LEN];
#define LOOPMAX 1000

int ETH_test::initial_state(GCFEvent& e, GCFPortInterface& /*port*/)
{
  int status = GCFEvent::HANDLED; // event not handled

  switch (e.signal)
  {
      case F_INIT:
      {
#if 0
	  // we want F_DATAIN's
	  channel.enableSig(F_DATAIN);
#endif

	  // open the ctrl_ port
	  channel.open();

	  // initialize buffer
	  memset(ETH_packet, 0, ETH_DATA_LEN);
      }
      break;
      
      case F_CONNECTED:
      {
	  cout << "connected" << endl;

	  channel.setTimer((long)0, 0, 0, 1);
      }
      break;

      case F_DATAIN:
      {
	  int n;

	  if ((n = channel.recv(ETH_packet, ETH_DATA_LEN)) < 0)
	  {
	      perror("channel.recv");
	      exit(EXIT_FAILURE);
	  }
	  if (n == 0) status = GCFEvent::ERROR;

	  cout << "Received " << n << " bytes." << endl;
      }
      break;

      //case F_DATAOUT:
      case F_TIMER:
      {
	  unsigned long* hdr = (unsigned long*)&ETH_packet[0];
	  strncpy((char*)hdr, "packet start\0", sizeof("packet start\0"));

	  char* cptr = (char*)&ETH_packet[ETH_DATA_LEN];
	  cptr -= 8;
	  strcpy(cptr, "the end");

	  // delay loop
	  //for (int i=0; i<60000; i++) { int j = i*2; j = j; }

	  DataEvent data;
	  data.payload.setBuffer(ETH_packet, ETH_DATA_LEN);
	  
	  channel.send(data);
      }
      break;
      case F_DISCONNECTED:
      {
	  // reopen the port
	  channel.open();
      }
      break;

      default:
	  status = GCFEvent::NOT_HANDLED;
	  break;
  }
  
  return status;
}

//
// Program usage
//
void usage(const char* progname)
{
  fprintf(stderr, "usage: %s\n"
	  "\t--if=<e.g. eth0>\n"
	  "\t[--client]\n"
	  "\t[--peer=<e.g. 00:11:22:33:44:55>]\n"
	  "\t[-h]\n",
	  progname);
}

//
// Contoller main function
//
int main(int argc, char* argv[])
{
  bool isClient = false;
  char ifname[MAX_IFNAME_LEN + 1];
  char dest_mac[HWADDR_LEN + 1];

  char progname[100];

  GCFTask::init(argc, argv);

  // remember program name
  strncpy(progname, argv[0], 100);

  dest_mac[0] = '\0';
  ifname[0] = '\0';

  while (1)
  {
    static struct option long_options[] = 
      {
	{ "client",       no_argument, 0, 'c' },
	{ "if",     required_argument, 0, 'i' },
        { "peer",   required_argument, 0, 'p' },
	{ "help",         no_argument, 0, 'h' },
	{ 0, 0, 0, 0 },
      };

    int option_index = 0;
    int c = getopt_long_only(argc, argv,
			     "ci:p:h", long_options, &option_index);
    
    if (c == -1) break;
    
    switch (c)
    {
      case 'c':
	isClient = true;
	break;
	
      case 'i':
	strncpy(ifname, optarg, MAX_IFNAME_LEN);
	break;
	
      case 'p':
	memset(dest_mac, 0, HWADDR_LEN + 1);
	strncpy(dest_mac, optarg, HWADDR_LEN);
	break;

      case 'h':
	usage(argv[0]);
	exit(EXIT_SUCCESS);
	break;

      default:
	printf ("?? getopt returned character code 0%o ??\n", c);
	break;
    }
  }

  if (optind < argc) {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      printf ("%s ", argv[optind++]);
    printf ("\n");
  }
  
  if (ifname[0] == '\0')
  {
    cerr << "Error: mandatory '--if' argument missing." << endl;
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  ETH_test server("ETH_test", isClient, ifname, dest_mac);
  server.start();

  GCFTask::run();

  cerr << "Returned from GCFTask::run()! should never happen." << endl;

  return 1;
}
