#include "lofar_config.h"

#include <MessageBus/MsgBus.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_QPID
#include <qpid/types/Exception.h>

using namespace qpid::messaging;
#endif

using std::string;

namespace LOFAR {

#ifdef HAVE_QPID
  static Duration TimeOutDuration(double secs)
  {
    if (secs > 0.0)
      return (Duration)(1000.0 * secs);

    return Duration::FOREVER;
  }
#endif


  FromBus::FromBus(const std::string &address, const std::string &options, const std::string &broker)
#ifdef HAVE_QPID
  try:
    itsBrokerName(broker),
    itsQueueName(address),
    itsConnection(broker),
    itsNrMissingACKs(0)
  {
    itsConnection.open();

    itsSession = itsConnection.createSession();

    Address addr(address+options);
    receiver = itsSession.createReceiver(addr);
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }
#else
  :
    itsBrokerName(broker),
    itsQueueName(address),
    itsNrMissingACKs(0)
  {
  }
#endif

 
  FromBus::~FromBus(void)
  {
    if (itsNrMissingACKs)
      LOG_ERROR_STR("Queue " << itsQueueName << " on broker " << itsBrokerName << " has " << itsNrMissingACKs << " messages not ACK'ed ");
  }

  bool FromBus::getString( std::string &str, double timeout) // timeout 0.0 means blocking
  {
#ifdef HAVE_QPID
    Message msg;

    bool ret = getMessage(msg, timeout);
    if (ret) {
        itsNrMissingACKs ++;
        str = msg.getContent();
    }

    return ret;
#else
    return false;
#endif
  }

#ifdef HAVE_QPID
  bool FromBus::getMessage(Message & msg, double timeout) // timeout 0.0 means blocking
  {
    bool ret= receiver.fetch(msg, TimeOutDuration(timeout));
    if (ret) itsNrMissingACKs++;
    return ret;
  }
#endif

  void FromBus::ack(void)
  {
#ifdef HAVE_QPID
     itsSession.acknowledge();
#endif
     itsNrMissingACKs --;
  }
  
 ToBus::ToBus(const std::string &address, const std::string &options, const std::string &broker) 
#ifdef HAVE_QPID
  try:
    itsBrokerName(broker),
    itsQueueName(address),
    itsConnection(broker)
  {
     itsConnection.open();

     itsSession = itsConnection.createSession();

     Address addr(address+options);
     sender = itsSession.createSender(addr);
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }
#else
  :
    itsBrokerName(broker),
    itsQueueName(address)
  {
  }
#endif

  ToBus::~ToBus(void)
  {
  }

  void ToBus::send(const std::string &msg)
  {
#ifdef HAVE_QPID
    Message tosend(msg);
    sender.send(tosend,true);
#endif
  }

  MultiBus::MultiBus(const std::string &broker) 
#ifdef HAVE_QPID
  try: 
    itsBrokerName(broker),
    itsConnection(broker),
    itsNrMissingACKs(0)
  {
     itsConnection.open();
     itsSession = itsConnection.createSession();
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }
#else
  :
    itsBrokerName(broker),
    itsNrMissingACKs(0)
  {
  }
#endif

  MultiBus::~MultiBus()
  {
    // fixme: memory leak for workers. Also infinite loop needs a fix.
  }

  void MultiBus::addQueue(MsgHandler handler, const std::string &address, const std::string &options)
  {
#ifdef HAVE_QPID
    Address addr(address+options);
    Receiver receiver = itsSession.createReceiver(addr);
    receiver.setCapacity(1);

    MsgWorker worker;
    worker.handler = handler;
    worker.queuename = string(address);
    itsHandlers[receiver] = worker;
#endif
  }

  void MultiBus::handleMessages(void)
  {
#ifdef HAVE_QPID
     while (1)
     {
        Receiver nrec = itsSession.nextReceiver();
        MsgWorker worker = itsHandlers[nrec];
        Message msg;
        nrec.get(msg);

        const std::string msgStr = msg.getContent();

        ASSERTSTR(worker.handler, "Undefined handler for queue " << worker.queuename);

        if (worker.handler(msgStr, worker.queuename))
          itsSession.acknowledge(msg);
        else // todo: define a proper fail over mechanism with proper handling
          itsSession.reject(msg); // broker can be configured for this
     }
#endif
  }

#ifdef HAVE_QPID
  bool MultiBus::getMessage(Message &msg, double timeout)
  {
    Receiver nrec = itsSession.nextReceiver();
    return nrec.get(msg, TimeOutDuration(timeout));
  }
#endif

  bool MultiBus::getString(std::string &str, double timeout)
  {
#ifdef HAVE_QPID
    Message msg;
    bool ret = getMessage(msg, timeout);

    if (ret) 
      str = msg.getContent();
    return ret;
#else
    return false;
#endif
  }

} // namespace LOFAR

