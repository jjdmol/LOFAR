#include <lofar_config.h>

#include <Messaging/FromBus.h>
#include <Messaging/ToBus.h>
#include <Messaging/LofarMessages.h>
#include <Common/LofarLogger.h>

// #include <qpid/messaging/Message.h>    //# for debuggging
// #include <qpid/messaging/Address.h>    //# for debuggging

#include <UnitTest++.h>

#include <iostream>
#include <memory>

using namespace LOFAR::Messaging;
using namespace std;

// //# for debugging
// void show(const Message& msg)
// {
//   cout << "ReplyTo       = " << msg.getReplyTo().str() << endl;
//   cout << "Subject       = " << msg.getSubject() << endl;
//   cout << "ContentType   = " << msg.getContentType() << endl;
//   cout << "MessageId     = " << msg.getMessageId() << endl;
//   cout << "UserId        = " << msg.getUserId() << endl;
//   cout << "CorrelationId = " << msg.getCorrelationId() << endl;
//   cout << "Priority      = " << msg.getPriority() << endl;
//   cout << "Ttl           = " << msg.getTtl().getMilliseconds() << endl;
//   cout << "Durable       = " << msg.getDurable() << endl;
//   cout << "Redelivered   = " << msg.getRedelivered() << endl;
//   cout << "Properties    = " << msg.getProperties() << endl;
//   cout << "Content       = " << msg.getContent() << endl;
//   cout << "ContentBytes  = " << msg.getContentBytes() << endl;
//   cout << "ContentObject = " << msg.getContentObject() << endl;
//   cout << "ContentPtr    = " << msg.getContentPtr() << endl;
//   cout << "ContentSize   = " << msg.getContentSize() << endl;
// }

struct BusFixture 
{
  BusFixture() :
    fromBus("testqueue", "{create: always, delete: always}"),
    toBus  ("testqueue"),
    timeOut(0.5)
    {}
  FromBus fromBus;
  ToBus   toBus;
  double  timeOut;
};

TEST_FIXTURE(BusFixture, EventMessage)
{
  cout << "** EventMessage **" << endl;
  EventMessage sendMsg;
  toBus.send(sendMsg);
  auto_ptr<Message> recvMsg(fromBus.getMessage(timeOut));
  fromBus.ack(*recvMsg);
  CHECK(sendMsg.type() == recvMsg->type());
  // show(sendMsg);
  // show(*recvMsg);
}

TEST_FIXTURE(BusFixture, MonitoringMessage)
{
  cout << "** MonitoringMessage **" << endl;
  MonitoringMessage sendMsg;
  toBus.send(sendMsg);
  auto_ptr<Message> recvMsg(fromBus.getMessage(timeOut));
  fromBus.ack(*recvMsg);
  CHECK(sendMsg.type() == recvMsg->type());
}

TEST_FIXTURE(BusFixture, ProgressMessage)
{
  cout << "** ProgressMessage **" << endl;
  ProgressMessage sendMsg;
  toBus.send(sendMsg);
  auto_ptr<Message> recvMsg(fromBus.getMessage(timeOut));
  fromBus.ack(*recvMsg);
  CHECK(sendMsg.type() == recvMsg->type());
}

TEST_FIXTURE(BusFixture, ServiceMessage)
{
  cout << "** ServiceMessage **" << endl;
  ServiceMessage sendMsg;
  toBus.send(sendMsg);
  auto_ptr<Message> recvMsg(fromBus.getMessage(timeOut));
  fromBus.ack(*recvMsg);
  CHECK(sendMsg.type() == recvMsg->type());
}


int main()
{
  INIT_LOGGER("tMessaging");
  return UnitTest::RunAllTests() > 0;
}
