//
//  Ping.cc: Implementation of the Ping task class.
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
//

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include "Ping.h"
#include "Echo_Protocol.ph"


namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

/**
 * Function to calculate the elapsed time between two tiemval's.
 */
static double time_elapsed(timeval* start, timeval* stop) 
{
  return stop->tv_sec-start->tv_sec 
    + (stop->tv_usec-start->tv_usec)/(double)1e6; 
}

Ping::Ping(string name)
  : GCFTask((State)&Ping::initial, name), ping_timer(-1)
{
  // register the port for debug tracing
  registerProtocol(ECHO_PROTOCOL, ECHO_PROTOCOL_STRINGS);

  /**
   * Initialize the "client" port
   * - Pass the this pointer, because this port belongs to
   *   this task.
   * - Give it the name "client".
   * - This is a Service Access Port which uses the
   *   ECHO_PROTOCOL 
   */
  client.init(*this, "EchoServer:test", GCFPortInterface::SAP, ECHO_PROTOCOL);
}

Ping::~Ping()
{
  cout << "Ping stopped." << endl;
}

GCFEvent::TResult Ping::initial(GCFEvent& e, GCFPortInterface& /*port*/)
{
	LOG_DEBUG_STR("Ping::initial: " << eventName(e));

  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_INIT:
      break;

    case F_ENTRY:
      client.open();
      break;

    case F_CONNECTED:
      
      // start ping_timer
      // - after 1 second
      // - every 2 seconds
      ping_timer = client.setTimer(1.0, 0.5);

      TRAN(Ping::connected);
      break;

    case F_DISCONNECTED:
      (void)client.setTimer(1.0); // try connect again after 1 second
      break;

    case F_TIMER:
      client.open();
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult Ping::connected(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("Ping::connected: " << eventName(e));

  GCFEvent::TResult status = GCFEvent::HANDLED;

  static int seqnr = 0;

  switch (e.signal)
  {
    case F_TIMER:
    {
  
    	timeval ping_time;
    
    	// create PingEvent
    	gettimeofday(&ping_time, 0);
    	EchoPingEvent ping;
      
      ping.seqnr = seqnr++;
      ping.ping_time = ping_time;
    	// send the event
    	client.send(ping);
    
    	cout << "PING sent (seqnr=" << ping.seqnr << ")" << endl;


    	TRAN(Ping::awaiting_echo); // wait for the echo
      break;
    }    

    case F_DISCONNECTED:
      //(void)client.open(); // try to reopen
      (void)client.cancelTimer(ping_timer);

      seqnr = 0;
      p.close();      
      TRAN(Ping::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult Ping::awaiting_echo(GCFEvent& e, GCFPortInterface& p)
{
	LOG_DEBUG_STR("Ping::awaiting_echo: " << eventName(e));

  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
  {
    case F_TIMER:
      cout << "Missed echo dead-line." << endl;
      break;

    case ECHO_ECHO:
    {
    	timeval echo_time;
    	gettimeofday(&echo_time, 0);
    
    	EchoEchoEvent echo(e);
    
    	cout << "ECHO received (seqnr=" << echo.seqnr << "): elapsed = "
    	     << time_elapsed(&(echo.ping_time), &echo_time) << " sec."<< endl;
      if (echo.seqnr == 600)
      {
        GCFScheduler::instance()->stop();
      }
      else
      {
    	  TRAN(Ping::connected);
      }
      break;
    }

    case F_DISCONNECTED:
      (void)client.cancelTimer(ping_timer);
      p.close();      
      TRAN(Ping::initial);
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }
  
  return status;
}

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
using namespace LOFAR::GCF::TM;

int main(int argc, char* argv[])
{
  GCFScheduler::instance()->init(argc, argv);

  Ping ping_task("PING");

  ping_task.start(); // make initial transition

  GCFScheduler::instance()->run();

  return 0;
}
