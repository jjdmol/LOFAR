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
#include <string>

#include <Common/LofarLogger.h>
#include <MessageBus/Message.h>
//#include <UnitTest++.h>


//using namespace qpid::messaging;
using namespace LOFAR;
using namespace std;

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

//**********************
// Creates a valid xml message to test against

string VALID_LOFAR_XML = \
"<message>"
"  <header>"
"    <system>LOFAR< / system>"
"    <version>1.0.0< / version>"
"    <protocol>"
"      <name>lofar.observation.start< / name>"
"      <version>1.0< / version>"
"    < / protocol>"
"    <source>"
"      <name>mySubSystem< / name>"
"      <user>user< / user>"
"      <uuid>< / uuid>"
"      <timestamp>< / timestamp>"
"      <summary>some test message< / summary>"
"    < / source>"
"    <ids>"
"      <momid>12345< / momid>"
"      <sasid>44883< / sasid>"
"    < / ids>"
"  < / header>"
"  <payload>"
"Observation.Correlator.channelWidth = 3051.7578125"
"Observation.Correlator.channelsPerSubband = 64"
"Observation.Correlator.integrationInterval = 1.00139008"
"Observation.DataProducts.Output_Correlated_[0].SAP = 0"
"Observation.DataProducts.Output_Correlated_[0].centralFrequency = 115039062.500000"
"Observation.DataProducts.Output_Correlated_[0].channelWidth = 3051.757812"
"Observation.DataProducts.Output_Correlated_[0].channelsPerSubband = 64"
"Observation.DataProducts.Output_Correlated_[0].duration = 119.165420"
"Observation.DataProducts.Output_Correlated_[0].fileFormat = AIPS++ / CASA"
"Observation.DataProducts.Output_Correlated_[0].filename = L257915_SAP000_SB000_uv.MS"
"Observation.DataProducts.Output_Correlated_[0].integrationInterval = 1.001390"
"Observation.DataProducts.Output_Correlated_[0].location = locus001: / data / L257915 /"
"Observation.DataProducts.Output_Correlated_[0].percentageWritten = 100"
"Observation.DataProducts.Output_Correlated_[0].size = 268083200"
"Observation.DataProducts.Output_Correlated_[0].startTime = 2015 - 01 - 15 09 : 32 : 09"
"Observation.DataProducts.Output_Correlated_[0].stationSubband = 77"
"Observation.DataProducts.Output_Correlated_[0].subband = 0"
"  < / payload>"
"< / message>";


struct ValidXMLFixture
{
  Message msg;

  ValidXMLFixture()
  :
  msg(VALID_LOFAR_XML)
  {
  }

  ~ValidXMLFixture() {}
};


int main(int argc, char* argv[]) 
{
	if (argc != 1) {
		cout << "Syntax: " << argv[0] << endl;
		return (1);
	}

  ValidXMLFixture fixture;

  cout << fixture.msg ;

#if HAVE_LIBXML2
  cout << "We have libxml installed" << endl;
#endif

  
/*
	Message	msg1("mySubSystem", "user", "some test message", "lofar.observation.start", "1.0", "12345", "54321");

	qpid::messaging::Message	qpMsg("Qpid message");
	Message		msg2(qpMsg);
	
	string	KVmapje("abc=[aap,noot,mies]\nmyInteger=5\nmyDouble=3.14");
	msg1.setTXTPayload(KVmapje);

	cout << msg1;*/

	return (0);
}

