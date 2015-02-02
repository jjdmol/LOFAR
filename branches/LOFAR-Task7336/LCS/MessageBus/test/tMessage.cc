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
#include <MessageBus/Message.h>

//using namespace qpid::messaging;
using namespace LOFAR;

#if 0
void showMessage(Message&	msg)
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

void compareMessages(Message&	lhm, Message& rhm)
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
#endif

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Syntax: " << argv[0] << " messagebus" << endl;
		return (1);
	}

	Message	msg1("mySubSystem", "user", "some test message", "lofar.observation.start", "1.0");

	qpid::messaging::Message	qpMsg("Qpid message");
	Message		msg2(qpMsg);
	
	string	KVmapje("abc=[aap,noot,mies]\nmyInteger=5\nmyDouble=3.14");
	msg1.setTXTPayload(KVmapje);



	return (0);
}

