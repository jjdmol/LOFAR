//
//  Ping.h: Definition of the Ping task class.
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

#ifndef _PING_H_
#define _PING_H_

#include <GCF/TM/GCF_Control.h>
#include <sys/time.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

/**
 * The Ping task sends ECHO_PING events to the Echo task and expects 
 * ECHO_ECHO events in return.
 */
class Ping : public GCFTask
{
 public:

  /**
   * The constructor for the Ping task.
   * @param name The name of this task. This name is used for looking up
   * connection establishment information from the FNameService and FAppTopology
   * configuration files.
   */
  Ping (string name);

  ~Ping();
  /**
   * Handler for the initial state. This handler is passed to the constructor
   * of the FTask class to indicate that it should be sent the F_INIT signal
   * which indicates the start of the state-machine.
   * @return FEvent::HANDLED or FEvent::NOT_HANDLED to indicate whether the
   * event has been handled or not.
   */
  GCFEvent::TResult initial      (GCFEvent& e, GCFPortInterface& p);

  /**
   * The "connected" state is reached when the Ping client has made a 
   * connection with the Echo server.
   */
  GCFEvent::TResult connected    (GCFEvent& e, GCFPortInterface& p);

  /**
   * The "awaiting_echo" state is reached when an ECHO_PING event has
   * been sent, but the ECHO_ECHO event has not been received yet.
   * When the ECHO_ECHO event has been received a transition to the
   * "connected" state is made.
   */
  GCFEvent::TResult awaiting_echo(GCFEvent& e, GCFPortInterface& p);

 private:

  /**
   * The Ping task is a client to the Echo task. The "client" port can be used
   * to send events to and receive events from the Echo task.
   */
  GCFPort client;

  /**
   * Ping messages are sent every 1 second. This variables holds the
   * timer_id of that timer to enable cancelation of the timer
   * in fault situations.
   */
  long ping_timer; // remember ping timer id to be able to cancel it
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
