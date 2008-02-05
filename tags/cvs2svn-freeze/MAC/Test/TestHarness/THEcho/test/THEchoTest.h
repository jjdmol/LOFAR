//
//  THEchoTest.h: Definition of the Echo task class.
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

#ifndef THEchoTest_H
#define THEchoTest_H

#include <GCF/TM/GCF_Control.h>

namespace THEchoTest
{
/**
 * The Echo task receives ECHO_PING events from the Ping task and
 * returns an ECHO_ECHO event for each ECHO_PING event received.
 */
class EchoTest : public GCFTask
{
 public:

  /**
   * The constructor for the Echo task.
   * @param name The name of this task. By differentiating in the name, multiple
   * instances of the same task can be created and addressed.
   */
  EchoTest(string name);
  ~EchoTest();

  /**
   * The initial state handler. This handler is passed to the FTask constructor
   * to indicate that the F_INIT event which starts the state machine is handled
   * by this handler.
   * @param e The event that was received and needs to be handled by the state
   * handler.
   * @param p The port interface (see @a FPortInterface) on which the event
   * was received.
   */
  GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);

  GCFEvent::TResult test1(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test2(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test3(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test4(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test5(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test6(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test7(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test8(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test9(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test10(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test11(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test12(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test13(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test14(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test15(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test16(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test17(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult test18(GCFEvent& e, GCFPortInterface& p);
  GCFEvent::TResult final(GCFEvent& e, GCFPortInterface& p);

private:
  bool _isClient(GCFPortInterface& p) const;
  bool _isRoutingClient(GCFPortInterface& p) const;
  bool _isServer(GCFPortInterface& p) const;
  void _send(GCFEvent& e, GCFPortInterface& p);

  /**
   * The Echo task acts as a server. Events that are received on the _server port
   * are replied through the same server port. Events that are received on the 
   * _routingServer port are replied through the _client port. 
   */
  GCFPort       _client;
  GCFPort       _routingClient;
  GCFPort       _server;
  unsigned int  _seqnr;
};

}; // namespace THEchoTest

#endif
