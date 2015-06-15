//#  Copyright (C) 2015
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <UnitTest++.h>
#include <MessageBus/MsgBus.h>

using namespace qpid::messaging;
using namespace LOFAR;

// Send a message containing FOO
void sendFOO() {
  LOFAR::MessageContent content;
  content.payload = "FOO";

  ToBus tb("tMsgBus");
  tb.send(content);
}

TEST(BrokerRunning) {
  Connection conn("amqp:tcp:127.0.0.1:5672");

  conn.open();
  conn.close();
}

TEST(ConstructReceiver) {
  FromBus fb("tMsgBus");
}

TEST(ConstructSender) {
  ToBus tb("tMsgBus");
}

TEST(SendReceive) {
  // Send
  sendFOO();

  // Receive
  LOFAR::Message msgReceived;

  FromBus fb("tMsgBus");
  CHECK(fb.getMessage(msgReceived, 1.0));

  MessageContent content(msgReceived.qpidMsg());
  CHECK_EQUAL("FOO", content.payload.get());

  fb.ack(msgReceived);
}

TEST(Ack) {
  // Send
  sendFOO();

  // Reeive
  LOFAR::Message msgReceived;

  FromBus fb("tMsgBus");
  CHECK(fb.getMessage(msgReceived, 0.2));

  fb.ack(msgReceived);

  // ACK = Accept, do NOT send again
  CHECK(!fb.getMessage(msgReceived, 0.2));
}

TEST(NAck) {
  // Send
  sendFOO();

  // Reeive
  LOFAR::Message msgReceived;

  FromBus fb("tMsgBus");
  CHECK(fb.getMessage(msgReceived, 0.2));

  fb.nack(msgReceived);

  // NACK = Send again
  CHECK(fb.getMessage(msgReceived, 0.2));

  // Keep queue clean
  fb.ack(msgReceived);
}

int main() {
  INIT_LOGGER("tMsgBus");

  MessageBus::init();

  return UnitTest::RunAllTests() > 0;
}

