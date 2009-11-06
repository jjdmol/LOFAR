// 
//  TCP_test.cc: test code for explicit FTCPPort
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
#include "imp/FTCPPort.h"

#include "Timer.h"

#define UDP_OVERHEAD (42) // UDP standard
#define TCP_OVERHEAD (82) // TCP standard

//#define MTU_LENGTH 1500   // standard network device
//#define MTU_LENGTH 16436  // for loopback device
#define MTU_LENGTH (9000) // Jumbo packets over Gigabit ethernet

class TCP_test : public FTask
{
 public:

  TCP_test(const char* name, bool isClient,
	   int port, char* hostname);

 private:

  int initial_state  (FEvent& e, FPortInterface& port);

 private:

  FTCPPort channel;

  unsigned long transferred;
  Timer timer;
  unsigned long startseqnr;
  unsigned long seqnr;
  unsigned long loss;

  long timerid;

  int itsPort;
};

TCP_test::TCP_test(const char* name, bool isClient,
		   int port, char* hostname) :
  FTask((State)&TCP_test::initial_state, name), channel(),
  transferred(0), startseqnr(0), seqnr(0), loss(0), timerid(0), itsPort(port)
{
  // client or server?
  channel.init(this, "channel", (isClient ? FPort::SAP : FPort::SPP));

  FPeerAddr addr(name, hostname, "channel", "TCP", port);

  channel.setAddr(addr);

  timer.start();
}

#define TCPBUFSIZE (MTU_LENGTH - TCP_OVERHEAD)
static char TCP_packet[TCPBUFSIZE];
#define LOOPMAX 512

int TCP_test::initial_state(FEvent& e, FPortInterface& /*port*/)
{
  int status = FEvent::HANDLED; // event not handled

  switch (e.signal)
    {
    case F_INIT_SIG:
      {
	// open the ctrl_ port
	channel.open();

	// we want F_DATAIN_SIG's
	channel.enableSig(F_DATAIN_SIG);

	// make sure we receive F_DATAOUT_SIG's
	if (channel.getType() == FPortInterface::SAP)
	{
	  channel.enableSig(F_DATAOUT_SIG);
	  
	  cout << "Connecting to port " << itsPort << endl;
	}
	else
	{
	    seqnr = (unsigned long)-1;
	    cout << "Listening on port " << itsPort
		 << " for connections..." << endl;
	}
	
	// initialize buffer
	memset(TCP_packet, 1, TCPBUFSIZE);
      }
      break;
      
    case F_CONNECTED_SIG:
      {
	ACE_DEBUG((LM_DEBUG, "connected\n"));

	// start statistics update timer once every 5 seconds
	timerid = channel.setTimer((long)5, 0, 5, 0);
      }
      break;

    case F_DATAIN_SIG:
      {
	int n;
	unsigned long* hdr = (unsigned long*)&TCP_packet[0];

	if ((n = channel.recv(TCP_packet, TCPBUFSIZE)) < 0)
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
      }
      break;

    case F_TIMER_SIG:
      {
	  timer.stop();
	  
	  double MBytes = transferred/(1024.0*1024.0);
	  cout << "Transferred " << MBytes << " MBytes; ";
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
      break;

    case F_DATAOUT_SIG:
//    case F_TIMER_SIG:
      {
	int n;

	unsigned long* hdr = (unsigned long*)&TCP_packet[0];
	*hdr = seqnr++;
	hdr++;
	strncpy((char*)hdr, "packet start\0", sizeof("packet start\0"));

	char* cptr = (char*)&TCP_packet[TCPBUFSIZE-1];
	cptr -= 8;
	strcpy(cptr, "the end");

	// delay loop
	//for (int i=0; i<60000; i++) { int j = i*2; j = j; }

	if ((n = channel.send(FEvent(F_RAW_SIG), TCP_packet, TCPBUFSIZE)) < 0)
	{
	  perror("channel.send");
	  exit(EXIT_FAILURE);
	}

	transferred += n;

	//cout << "channel.send: sent " << n << " bytes." << endl;
      }
      break;


    case F_DISCONNECTED_SIG:
      {
	// stop the statistics update timer
	channel.cancelTimer(timerid);

	// reopen the port
	channel.open();
      }
      break;

    default:
	status = FEvent::NOT_HANDLED;
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
		  "\t[--host <hostname=localhost>]\n",
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
    else if (0 == ACE_OS::strcmp(current_arg, "--help"))
    {
      usage(argv[0]);
      exit(EXIT_SUCCESS);
    }

    arg.consume_arg();
  }

  TCP_test server("TCP_test", isClient, port, host);
  server.start();

  FTask::run();

  ACE_ERROR((LM_ERROR,
     "Returned from FTask::run()! Should never happen.\n"));

  return 1;
}
