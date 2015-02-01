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

    itsSession = itsConnection.createSession();

    Address addr(address+options);
    Receiver receiver = itsSession.createReceiver(addr);
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }

 
  FromBus::~FromBus(void)
  {
    if (itsNrMissingACKs) {
//      LOG_ERROR_STR("Queue " << itsQueueName << " on broker " << itsBrokerName << " has " << itsNrMissingACKs << " messages not ACK'ed ");
	}
  }

  bool FromBus::getMessage(Message &msg, double timeout) // timeout 0.0 means blocking
  {
    Receiver next;
	cout << "waiting for message..." << endl;
    if (itsSession.nextReceiver(next,TimeOutDuration(timeout)))
    {
		cout << "message available..." << endl;
        itsNrMissingACKs++;
        return next.get(msg);
    }
    return false;
  }

  void FromBus::nack(Message &msg)
  {
     itsSession.release(msg);
     itsNrMissingACKs--;
  }

  void FromBus::ack(Message &msg)
  {
     itsSession.acknowledge(msg);
     itsNrMissingACKs --;
  }

  void FromBus::reject(Message &msg)
  {
     itsSession.reject(msg);
     itsNrMissingACKs --;
  }

  bool FromBus::addQueue(const std::string &address, const std::string &options)
  {
     try
     {
        Address addr(address+options);
        Receiver receiver = itsSession.createReceiver(addr);
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
     itsSession = itsConnection.createSession();
     Address addr(address+options);
     Sender itsSender = itsSession.createSender(addr);
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }
  ToBus::~ToBus(void)
  {
  }

  void ToBus::send(const std::string &msg)
  {
    Message tosend(msg);
    itsSender.send(tosend,true);
  }

} // namespace LOFAR

#endif
