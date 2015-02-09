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
#include <MessageBus/MsgBus.h>

using namespace qpid::messaging;
using namespace LOFAR;


void showMessage(qpid::messaging::Message&	msg)
{
	cout << "Message ID    : " << msg.getMessageId() << endl;
	cout << "User ID       : " << msg.getUserId() << endl;
	cout << "Correlation ID: " << msg.getCorrelationId() << endl;
	cout << "Subject       : " << msg.getSubject() << endl;
	cout << "Reply to      : " << msg.getReplyTo() << endl;
	cout << "Content type  : " << msg.getContentType() << endl;
	cout << "Priority      : " << msg.getPriority() << endl;
//	cout << "TTL           : " << msg.getTtl() << endl;
	cout << "Durable       : " << (msg.getDurable() ? "Yes" : "No") << endl;
	cout << "Redelivered   : " << (msg.getRedelivered() ? "Yes" : "No")  << endl;
	cout << "Properties    : " << msg.getProperties() << endl;
	cout << "Content size  : " << msg.getContentSize() << endl;
	cout << "Content       : " << msg.getContent() << endl;
}

void compareMessages(qpid::messaging::Message&	lhm, qpid::messaging::Message& rhm)
{
	ASSERTSTR(lhm.getMessageId()     == rhm.getMessageId(),     "messageIDs differ");
	ASSERTSTR(lhm.getUserId()        == rhm.getUserId(),        "UserIDs differ");
	ASSERTSTR(lhm.getCorrelationId() == rhm.getCorrelationId(), "CorrelationIDs differ");
	ASSERTSTR(lhm.getSubject()       == rhm.getSubject(),       "Subjects differ");
	ASSERTSTR(lhm.getReplyTo()       == rhm.getReplyTo(),       "ReplyTos differ");
	ASSERTSTR(lhm.getContentType()   == rhm.getContentType(),   "ContentTypes differ");
	ASSERTSTR(lhm.getPriority()      == rhm.getPriority(),      "Priorities differ");
	ASSERTSTR(lhm.getTtl()           == rhm.getTtl(),           "TTLs differ");
	ASSERTSTR(lhm.getDurable()       == rhm.getDurable(),       "Durability differs");
	ASSERTSTR(lhm.getRedelivered()   == rhm.getRedelivered(),   "Redelivered differs");
//	ASSERTSTR(lhm.getProperties()    == rhm.getProperties(),    "Properties differ");
	ASSERTSTR(lhm.getContentSize()   == rhm.getContentSize(),   "ContentSize differs");
	ASSERTSTR(lhm.getContent()       == rhm.getContent(),       "Content differs");
}


int main(int argc, char* argv[]) {
  INIT_LOGGER("tMsgBus");

  MessageBus::init();

  std::string queue(argc == 2 ? argv[1] : "tMsgBus-test-queue");

  cout << "Using queue " << queue << " (Syntax: " << argv[0] << " messagebus)" << endl;

	cout << "--- Drain the queue ---" << endl;
	FromBus	fb(queue);
	LOFAR::Message	receivedMsg;
	while (fb.getMessage(receivedMsg, 0.01)) {
		fb.ack(receivedMsg);
	}

	cout << "--- TEST 1: create a message from a string, send it, receive it, print it. --- " << endl;
	ToBus	tb(queue);
	string	someText("An example message constructed from text");
	tb.send(someText);
	fb.getMessage(receivedMsg);
	fb.ack(receivedMsg);
	showMessage(receivedMsg.qpidMsg());

	cout << "--- TEST 2: create a message by hand, send it, receive it, print it, compare them. --- " << endl;
	LOFAR::Message		msg2Send;
	msg2Send.setTXTPayload("Manually constructed message");
	tb.send(msg2Send);
	fb.getMessage(receivedMsg);
	fb.ack(receivedMsg);
	showMessage(receivedMsg.qpidMsg());
	compareMessages(msg2Send.qpidMsg(), receivedMsg.qpidMsg());

	cout << "--- TEST 3: add an extra queue, send messages to both queues, receive them. --- " << endl;
	ToBus	tbExtra("extraTestQ");
	tbExtra.send("Message send to extra queue");
	tb.send("Message send to original queue");

	fb.addQueue("extraTestQ");
	fb.getMessage(receivedMsg);
    fb.ack(receivedMsg);
    showMessage(receivedMsg.qpidMsg());
    fb.getMessage(receivedMsg);
    fb.ack(receivedMsg);
    showMessage(receivedMsg.qpidMsg());

	cout << "--- All test successful! ---" << endl;

	return (0);
}

