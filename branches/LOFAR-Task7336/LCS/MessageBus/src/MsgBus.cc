#include "lofar_config.h"

#include <MessageBus/MsgBus.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_QPID
#include <qpid/types/Exception.h>
#include <qpid/messaging/exceptions.h>
//#include <qpid/messaging/Logger.h>

using namespace qpid::messaging;

namespace LOFAR {

  static Duration TimeOutDuration(double secs)
  {
    if (secs > 0.0)
      return (Duration)(1000.0 * secs);

    return Duration::FOREVER;
  }

  FromBus::FromBus(const std::string &address, const std::string &options, const std::string &broker)
  try:
    itsConnection(broker),
    itsNrMissingACKs(0)
  {
    itsConnection.open();
	cout << "Connected to: " << itsConnection.getUrl() << endl;

    itsSession = itsConnection.createSession();

    Address addr(address+options);
    Receiver receiver = itsSession.createReceiver(addr);
    receiver.setCapacity(1);
	cout << "Receiver started at queue: " << receiver.getName() << endl;
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }

 
  FromBus::~FromBus(void)
  {
    if (itsNrMissingACKs) {
//      LOG_ERROR_STR("Queue " << itsQueueName << " on broker " << itsBrokerName << " has " << itsNrMissingACKs << " messages not ACK'ed ");
	}

    try {
      // Make sure all requests are finished
      itsConnection.close();
    } catch(const qpid::types::Exception &ex) {
      LOG_FATAL_STR("Exception in destructor, cannot guarantee message delivery: " << ex.what());
    }
  }

  bool FromBus::getMessage(LOFAR::Message &msg, double timeout) // timeout 0.0 means blocking
  {
    Receiver next;
	qpid::messaging::Message	qmsg;
	cout << "waiting for message..." << endl;
    if (itsSession.nextReceiver(next,TimeOutDuration(timeout))) {
		cout << "message available on queue: " << next.getName() << endl;
        itsNrMissingACKs++;
        if (next.get(qmsg)) {
			msg = LOFAR::Message(qmsg);
			return true;
		}
    }
    return false;
  }

  void FromBus::ack(LOFAR::Message &msg)
  {
     itsSession.acknowledge(msg.qpidMsg());
     itsNrMissingACKs --;
  }

  void FromBus::nack(LOFAR::Message &msg)
  {
     itsSession.release(msg.qpidMsg());
     itsNrMissingACKs--;
  }

  void FromBus::reject(LOFAR::Message &msg)
  {
     itsSession.reject(msg.qpidMsg());
     itsNrMissingACKs --;
  }

  bool FromBus::addQueue(const std::string &address, const std::string &options)
  {
     try
     {
        Address addr(address+options);
        Receiver receiver = itsSession.createReceiver(addr);
        receiver.setCapacity(1);
		cout << "Receiver started at queue: " << receiver.getName() << endl;
     } catch(const qpid::types::Exception &ex) {
       //THROW(MessageBusException, ex.what());
       return false;
     }
     return true;
  }

  ToBus::ToBus(const std::string &address, const std::string &options, const std::string &broker) 
  try:
    itsConnection(broker)
  {
     itsConnection.open();
	 cout << "Connected to: " << itsConnection.getUrl() << endl;

     itsSession = itsConnection.createSession();
     Address addr(address+options);
     itsSender = itsSession.createSender(addr);
 	 cout << "Sender created: " << itsSender.getName() << endl;
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }

  ToBus::~ToBus(void)
  {

    try {
      // Make sure all requests are finished
      istSender.close();
      itsConnection.close();
    } catch(const qpid::types::Exception &ex) {
      LOG_FATAL_STR("Exception in destructor, cannot guarantee message delivery: " << ex.what());
    }
  }

  void ToBus::send(const std::string &msg)
  {
    LOFAR::Message tosend(msg);
    itsSender.send(tosend.qpidMsg(), true);
  }

  void ToBus::send(LOFAR::Message& msg)
  {
    itsSender.send(msg.qpidMsg(), true);
  }

} // namespace LOFAR

#endif
