#include "lofar_config.h"

#include <MessageBus/MsgBus.h>
#include <Common/LofarLogger.h>

#ifdef HAVE_QPID
#include <qpid/types/Exception.h>
#include <qpid/messaging/exceptions.h>
#include <qpid/messaging/Logger.h>

using namespace qpid::messaging;
#endif

namespace LOFAR {

#ifdef HAVE_QPID
  static Duration TimeOutDuration(double secs)
  {
    if (secs > 0.0)
      return (Duration)(1000.0 * secs);

    return Duration::FOREVER;
  }

  class QpidLogSink: qpid::messaging::LoggerOutput {
  public:
    virtual void log(Level level, bool user, const char* file, int line, const char* function, const std::string& message) {
      (void)user;

      switch(level) {
        case trace:
        case debug:
          LOG_DEBUG_STR("[QPID] " << function << " @ " << file << ":" << line << " " << message);
          break;

        case info:
        case notice:
          LOG_INFO_STR("[QPID] " << function << " @ " << file << ":" << line << " " << message);
          break;

        case warning:
          LOG_WARN_STR("[QPID] " << function << " @ " << file << ":" << line << " " << message);
          break;

        case error:
          LOG_ERROR_STR("[QPID] " << function << " @ " << file << ":" << line << " " << message);
          break;

        case critical:
          LOG_FATAL_STR("[QPID] " << function << " @ " << file << ":" << line << " " << message);
          break;
      }
    }
  };

  static QpidLogSink qpidLogSink;
#if 0
  void init() {
    Logger::setOutput(qpidLogSink);
  }

  void done() {
  }
#endif

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

  bool FromBus::getString(std::string &str, double timeout) // timeout 0.0 means blocking
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
  bool FromBus::getMessage(Message &msg, double timeout) // timeout 0.0 means blocking
  {
    bool ret= receiver.fetch(msg, TimeOutDuration(timeout));
    if (ret) itsNrMissingACKs++;
    return ret;
  }

  void FromBus::nack(Message &msg)
  {
    itsSession.release(msg);

     itsNrMissingACKs--;
  }
#endif

  void FromBus::ack(void)
  {
#ifdef HAVE_QPID
     itsSession.acknowledge();
#endif

     // acknowlegde covers ALL messages received so far
     itsNrMissingACKs = 0;
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

  void MultiBus::addQueue(const std::string &address, const std::string &options)
  {
#ifdef HAVE_QPID
    Address addr(address + options);
    Receiver receiver = itsSession.createReceiver(addr);
    receiver.setCapacity(1);

    itsReceivers[address] = receiver;
#endif
  }

#ifdef HAVE_QPID
  bool MultiBus::getMessage(Message &msg, double timeout)
  {
    try {
      Receiver nrec = itsSession.nextReceiver(TimeOutDuration(timeout));
      nrec.get(msg);

      return true;
    } catch(NoMessageAvailable&) {
      return false;
    }
  }

  void MultiBus::nack(Message &msg)
  {
    itsSession.release(msg);
  }
#endif

  void MultiBus::ack()
  {
#ifdef HAVE_QPID
    itsSession.acknowledge();
#endif
  }

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

