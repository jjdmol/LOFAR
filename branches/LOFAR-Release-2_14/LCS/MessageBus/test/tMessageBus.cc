//#  Copyright (C) 2015
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <UnitTest++.h>
#include <MessageBus/MessageBus.h>
#include <MessageBus/FromBus.h>
#include <MessageBus/ToBus.h>

using namespace qpid::messaging;
using namespace LOFAR;

// Send a message containing FOO
void sendFOO() {
  LOFAR::MessageContent content;
  content.payload = "FOO";

  ToBus tb("tMessageBus");
  tb.send(content);
}

// Receive a message containing FOO (and ACK  it)
LOFAR::Message receiveFOO() {
  LOFAR::Message msg;

  FromBus fb("tMessageBus");
  CHECK(fb.getMessage(msg, 1.0));

  MessageContent content(msg.qpidMsg());
  CHECK_EQUAL("FOO", content.payload.get());

  fb.ack(msg);

  return msg;
}

TEST(ConstructReceiver) {
  FromBus fb("tMessageBus");
}

TEST(ConstructSender) {
  ToBus tb("tMessageBus");
}

TEST(SendReceive) {
  sendFOO();
  receiveFOO();
}

TEST(Forward) {
  // Send
  sendFOO();

  // Receive
  LOFAR::Message msgReceived = receiveFOO();

  // Forward
  ToBus tb("tMessageBus-extraTestQ");
  tb.send(msgReceived);

  // Receive again
  LOFAR::Message msgReceived2;
  FromBus fb2("tMessageBus-extraTestQ");
  CHECK(fb2.getMessage(msgReceived2, 1.0));
  fb2.ack(msgReceived2);

  MessageContent content(msgReceived2.qpidMsg());
  CHECK_EQUAL("FOO", content.payload.get());
}

TEST(Ack) {
  // Send
  sendFOO();

  // Reeive
  receiveFOO();

  // ACK = Accept, do NOT send again
  FromBus fb("tMessageBus");
  LOFAR::Message msg;
  CHECK(!fb.getMessage(msg, 0.1));
}

TEST(NAck) {
  // Send
  sendFOO();

  // Reeive
  LOFAR::Message msg;

  FromBus fb("tMessageBus");
  CHECK(fb.getMessage(msg, 1.0));

  fb.nack(msg);

  // NACK = Send again
  CHECK(fb.getMessage(msg, 1.0));

  // Keep queue clean
  fb.ack(msg);
}

TEST(Reject) {
  // Send
  sendFOO();

  // Reeive
  LOFAR::Message msg;

  FromBus fb("tMessageBus");
  CHECK(fb.getMessage(msg, 1.0));

  fb.reject(msg);

  // Reject = DONT Send again
  CHECK(!fb.getMessage(msg, 0.1));
}

int main() {
  INIT_LOGGER("tMessageBus");

  MessageBus::init();

  /*
   * Test if the broker is running. If not, other tests will
   * block indefinitely, waiting for a broker.
   */

  // Need low-level routines to prevent the use of reconnect
  Connection conn("amqp:tcp:127.0.0.1:5672", "{reconnect: false}");

  conn.open();
  conn.close();

  // Run the tests
  return UnitTest::RunAllTests() > 0;
}

