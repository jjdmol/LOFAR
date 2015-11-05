#define DEBUG

// 
//  UDP_test.cc: test code for explicit FUDPPort
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

#include <ace/OS.h>
#include <ace/Arg_Shifter.h>
#include "FControl.h"
#include "imp/FUDPPort.h"

#include "Timer.h"

#define UDP_OVERHEAD (42) // UDP standard
//#define MTU_LENGTH 1500   // standard network device
//#define MTU_LENGTH 16436  // for loopback device
#define MTU_LENGTH (9000) // Jumbo packets over Gigabit ethernet

static double max_bandwidth = 0.5; // Mbytes/sec

class UDP_test : public FTask
{
 public:

  UDP_test(const char* name, bool isClient,
	   int port, char* hostname);

 private:

  int initial_state  (FEvent& e, FPortInterface& port);

 private:

  FUDPPort channel;

  unsigned long transferred;
  Timer timer;
  unsigned long startseqnr;
  unsigned long seqnr;
  unsigned long loss;
};

UDP_test::UDP_test(const char* name, bool isClient,
		   int port, char* hostname) :
  FTask((State)&UDP_test::initial_state, name), channel(),
  transferred(0), startseqnr(0), seqnr(0), loss(0)
{
  // client or server?
  channel.init(this, "channel", (isClient ? FPort::SAP : FPort::SPP));

  FPeerAddr addr(name, hostname, "channel", "UDP", port);

  channel.setAddr(addr);

  timer.start();
}

#define UDPBUFSIZE (MTU_LENGTH - UDP_OVERHEAD)
static char UDP_packet[UDPBUFSIZE];
#define LOOPMAX 5120

int UDP_test::initial_state(FEvent& e, FPortInterface& /*port*/)
{
  int status = FEvent::NOT_HANDLED; // event not handled

  switch (e.signal)
    {
    case F_INIT_SIG:
      {
	// we want F_DATAIN_SIG's
	channel.enableSig(F_DATAIN_SIG);

	// open the ctrl_ port
	channel.open();

	// make sure we receive F_DATAOU_SIG's
	if (channel.getType() == FPortInterface::SAP)
	{
#if 1
	  channel.enableSig(F_DATAOUT_SIG);
#else
	  int delay_usecs = (int)floor((((UDPBUFSIZE*1.0)/(1024.0*1024.0))/max_bandwidth)*1000000);
	  cout << "max_bandwidth=" << max_bandwidth << endl;
	  cout << "delay_usecs=" << delay_usecs << endl;
	  channel.setTimer(0,0,0,delay_usecs);
#endif
	}
	else seqnr = (unsigned long)-1;
	
	// initialize buffer
	memset(UDP_packet, 0, UDPBUFSIZE);

	status = FEvent::HANDLED;
      }
      break;
      
    case F_CONNECTED_SIG:
      {
	ACE_DEBUG((LM_DEBUG, "connected\n"));
	status = FEvent::HANDLED;
      }
      break;

    case F_DATAIN_SIG:
      {
	int n;
	unsigned long* hdr = (unsigned long*)&UDP_packet[0];

	status = FEvent::HANDLED;
	if ((n = channel.recv(UDP_packet, UDPBUFSIZE)) < 0)
	{
	  perror("channel.recv");
	  exit(EXIT_FAILURE);
	}
	if (n == 0) status = FEvent::ERROR;

	if (*hdr != seqnr+1)
	{
	  //cout << "hdr=" << *hdr << ", seqnr=" << seqnr << endl;
	  loss += *hdr - (seqnr + 1);
	  //cout << "loss=" << loss << endl;
	}
	seqnr = *hdr;

	transferred += n;

	if (seqnr - startseqnr >= LOOPMAX)
	{
	  timer.stop();
	  
	  double MBytes = transferred/(1024.0*1024.0);
	  cout << "Received " << MBytes << " MBytes; ";
	  cout << "Transfer rate = " << MBytes/timer.time_elapsed() <<
	    " MBytes/sec; ";
	  cout << "Lost " << loss << " packets (" << (loss*100.0)/(seqnr - startseqnr)
	       << "%)"
	       << (MBytes/timer.time_elapsed())*8 << " Mbits/sec"
	       << endl;
	  
	  timer.start();

	  startseqnr  = seqnr;
	  transferred = 0;
	  loss        = 0;
	}
      }
      break;

    case F_DATAOUT_SIG:
    case F_TIMER_SIG:
      {
	int n;

	unsigned long* hdr = (unsigned long*)&UDP_packet[0];
	*hdr = seqnr++;
	hdr++;
	strncpy((char*)hdr, "packet start\0", sizeof("packet start\0"));

	char* cptr = (char*)&UDP_packet[UDPBUFSIZE-1];
	cptr -= 8;
	strcpy(cptr, "the end");

	// delay loop
	//for (int i=0; i<60000; i++) { int j = i*2; j = j; }

	if ((n = channel.send(FEvent(F_RAW_SIG), UDP_packet, UDPBUFSIZE)) < 0)
	{
	  perror("channel.send");
	  exit(EXIT_FAILURE);
	}

	transferred += n;

	if (0 == (seqnr % LOOPMAX))
	{
	  timer.stop();
	  
	  double MBytes = transferred/(1024.0*1024.0);
	  cout << "Sent " << MBytes << " MBytes; ";
	  cout << "Transfer rate = " << MBytes/timer.time_elapsed() <<
	    " MBytes/sec" << endl;
	  
	  timer.start();

	  transferred = 0;
	}

	//cout << "channel.send: sent " << n << " bytes." << endl;
	status = FEvent::HANDLED;
      }
      break;
    case F_DISCONNECTED_SIG:
      {
	// reopen the port
	channel.open();
	status = FEvent::HANDLED;
      }
      break;
    }
  
  return status;
}

//
// Program usage
//
void usage(const char* progname)
{
  ACE_OS::fprintf(stderr, "usage: %s\n"
		  "\t[--client]\n"
		  "\t[--port <port=20011>]\n"
		  "\t[--host <hostname=localhost>]\n"
		  "\t[--bw <mbytes/sec=0.5>]\n",
		  progname);
}

//
// Contoller main function
//
#define MAX_HOSTNAME_LEN 100
int main(int argc, char* argv[])
{
  bool isClient = false;
  int port = 20011;
  char host[MAX_HOSTNAME_LEN + 1];

  ACE_OS::strncpy(host, "localhost", MAX_HOSTNAME_LEN);

  char progname[100];

  // remember program name
  strncpy(progname, argv[0], 100);

  ACE_Arg_Shifter arg(argc, argv);

  while (arg.is_anything_left())
  {
    char* current_arg = (char*)arg.get_current();

    if (0 == ACE_OS::strcmp(current_arg, "--client"))
    {
      isClient = true;
    }
    else if (0 == ACE_OS::strcmp(current_arg, "--port"))
    {
      arg.consume_arg();
      port = atoi(arg.get_current());
    }
    else if (0 == ACE_OS::strcmp(current_arg, "--host"))
    {
      arg.consume_arg();
      strncpy(host, arg.get_current(), MAX_HOSTNAME_LEN);
    }
    else if (0 == ACE_OS::strcmp(current_arg, "--bw"))
    {
      arg.consume_arg();
      max_bandwidth = atof(arg.get_current());
    }
    else if (0 == ACE_OS::strcmp(current_arg, "--help"))
    {
      usage(argv[0]);
      exit(EXIT_SUCCESS);
    }

    arg.consume_arg();
  }

  UDP_test server("UDP_test", isClient, port, host);
  server.start();

  FTask::run();

  ACE_ERROR((LM_ERROR,
     "Returned from FTask::run()! Should never happen.\n"));

  return 1;
}
