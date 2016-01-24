#include "lofar_config.h"

#include <MessageBus/ToBus.h>
#include <MessageBus/Exceptions.h>
#include "Util.h"

#include <Common/LofarLogger.h>

#ifdef HAVE_QPID
#include <qpid/types/Exception.h>
#include <qpid/messaging/exceptions.h>
#endif

using namespace qpid::messaging;

namespace LOFAR {

  ToBus::ToBus(const std::string &address, const std::string &options, const std::string &broker) 
  try:
    itsConnection(broker,"{reconnect:true}")
  {
    LOG_DEBUG_STR("[ToBus] Connecting to broker " << broker);
    itsConnection.open();
    LOG_INFO_STR("[ToBus] Connected to broker " << itsConnection.getUrl());

    itsSession = itsConnection.createSession();
    Address addr(queue_prefix()+address+options);
    itsSender = itsSession.createSender(addr);
    LOG_INFO_STR("[ToBus] Sender created at queue " << itsSender.getName());
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }

  ToBus::~ToBus(void)
  {

    try {
      // Make sure all requests are finished

      // connection.close() closes all senders, receivers, and sessions as well.
      itsConnection.close();
    } catch(const qpid::types::Exception &ex) {
      LOG_FATAL_STR("Exception in destructor, cannot guarantee message delivery: " << ex.what());
    }
  }

  void ToBus::send(const LOFAR::MessageContent &msg)
  {
    LOFAR::Message tosend(msg.qpidMsg());

    send(tosend);
  }

  void ToBus::send(LOFAR::Message& msg)
  {
    LOG_DEBUG_STR("[ToBus] Sending message to queue " << itsSender.getName() << ": " << msg.short_desc());
    itsSender.send(msg.qpidMsg(), true);
    LOG_DEBUG_STR("[ToBus] Message sent to queue " << itsSender.getName() << ": " << msg.short_desc());
  }

} // namespace LOFAR

