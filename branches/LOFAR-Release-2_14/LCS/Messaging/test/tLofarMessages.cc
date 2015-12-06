#include <lofar_config.h>

#include <Messaging/LofarMessages.h>
#include <Messaging/Exceptions.h>
#include <Common/LofarLogger.h>

#include <qpid/messaging/Message.h>

#include <UnitTest++.h>
#include <memory>

using namespace LOFAR::Messaging;
using namespace std;

struct QpidMsgFixture
{
  QpidMsgFixture() {
    qpidMsg.setProperty("SystemName", "LOFAR");
    qpidMsg.setProperty("MessageId", "1b4e28ba-2fa1-11d2-883f-b9a761bde3fb");
    qpidMsg.setProperty("MessageType", "EventMessage");
  }
  qpid::messaging::Message qpidMsg;
};

TEST_FIXTURE(QpidMsgFixture, UnknownMessageType)
{
  cout << "** UnknownMessageType **" << endl;
  qpidMsg.setProperty("MessageType", "FooBarMessage");
  CHECK_THROW(Message::create(qpidMsg), UnknownMessageType);
}

TEST_FIXTURE(QpidMsgFixture, WrongSystemName)
{
  cout << "** WrongSystemName **" << endl;
  qpidMsg.setProperty("SystemName", "NOTLOFAR");
  CHECK_THROW(Message::create(qpidMsg), InvalidMessage);
}

TEST_FIXTURE(QpidMsgFixture, InvalidMessageId)
{
  cout << "** InvalidMessageId **" << endl;
  qpidMsg.setProperty("MessageId", "invalid-uuid-string");
  CHECK_THROW(Message::create(qpidMsg), InvalidMessage);
}

SUITE(EventMessage)
{
  TEST(DefaultEventMessage)
  {
    cout << "** DefaultEventMessage ** " << endl;
    EventMessage msg;
    CHECK(msg.type() == "EventMessage");
    CHECK(msg.getProperty("SystemName") == "LOFAR");
    string messageId(msg.getProperty("MessageId"));
    CHECK(messageId.size() == 36 && 
          messageId != "00000000-0000-0000-0000-000000000000");
  }

  TEST_FIXTURE(QpidMsgFixture, QpidEventMessage)
  {
    cout << "** QpidEventMessage ** " << endl;
    qpidMsg.setProperty("MessageType", "EventMessage");
    auto_ptr<Message> msg(Message::create(qpidMsg));
    CHECK(dynamic_cast<EventMessage*>(msg.get()));
    CHECK(msg->type() == "EventMessage");
  }
}


SUITE(MonitoringMessage)
{
  TEST(DefaultMonitoringMessage)
  {
    cout << "** DefaultMonitoringMessage ** " << endl;
    MonitoringMessage msg;
    CHECK(msg.type() == "MonitoringMessage");
    CHECK(msg.getProperty("SystemName") == "LOFAR");
    string messageId(msg.getProperty("MessageId"));
    CHECK(messageId.size() == 36 && 
          messageId != "00000000-0000-0000-0000-000000000000");
  }

  TEST_FIXTURE(QpidMsgFixture, QpidMonitoringMessage)
  {
    cout << "** QpidMonitoringMessage ** " << endl;
    qpidMsg.setProperty("MessageType", "MonitoringMessage");
    auto_ptr<Message> msg(Message::create(qpidMsg));
    CHECK(dynamic_cast<MonitoringMessage*>(msg.get()));
    CHECK(msg->type() == "MonitoringMessage");
  }
}


SUITE(ProgressMessage)
{
  TEST(DefaultProgressMessage)
  {
    cout << "** DefaultProgressMessage ** " << endl;
    ProgressMessage msg;
    CHECK(msg.type() == "ProgressMessage");
    CHECK(msg.getProperty("SystemName") == "LOFAR");
    string messageId(msg.getProperty("MessageId"));
    CHECK(messageId.size() == 36 && 
          messageId != "00000000-0000-0000-0000-000000000000");
  }

  TEST_FIXTURE(QpidMsgFixture, QpidProgressMessage)
  {
    cout << "** QpidProgressMessage ** " << endl;
    qpidMsg.setProperty("MessageType", "ProgressMessage");
    auto_ptr<Message> msg(Message::create(qpidMsg));
    CHECK(dynamic_cast<ProgressMessage*>(msg.get()));
    CHECK(msg->type() == "ProgressMessage");
  }
}


SUITE(ServiceMessage)
{
  TEST(DefaultServiceMessage)
  {
    cout << "** DefaultServiceMessage ** " << endl;
    ServiceMessage msg;
    CHECK(msg.type() == "ServiceMessage");
    CHECK(msg.getProperty("SystemName") == "LOFAR");
    string messageId(msg.getProperty("MessageId"));
    CHECK(messageId.size() == 36 && 
          messageId != "00000000-0000-0000-0000-000000000000");
  }

  TEST_FIXTURE(QpidMsgFixture, QpidServiceMessage)
  {
    cout << "** QpidServiceMessage ** " << endl;
    qpidMsg.setProperty("MessageType", "ServiceMessage");
    auto_ptr<Message> msg(Message::create(qpidMsg));
    CHECK(dynamic_cast<ServiceMessage*>(msg.get()));
    CHECK(msg->type() == "ServiceMessage");
  }
}


int main()
{
  INIT_LOGGER("tLofarMessages");
  return UnitTest::RunAllTests() > 0;
}
