#include "lofar_config.h"

#include <MessageBus/FromBus.h>
#include <MessageBus/Exceptions.h>
#include "Util.h"

#include <Common/LofarLogger.h>

#ifdef HAVE_QPID
#include <qpid/types/Exception.h>
#include <qpid/messaging/exceptions.h>
#endif

using namespace qpid::messaging;

namespace LOFAR {
  FromBus::FromBus(const std::string &address, const std::string &options, const std::string &broker)
  try:
    itsConnection(broker,"{reconnect:true}"),
    itsNrMissingACKs(0)
  {
    LOG_DEBUG_STR("[FromBus] Connecting to broker " << broker);
    itsConnection.open();
    LOG_INFO_STR("[FromBus] Connected to broker " << itsConnection.getUrl());

    itsSession = itsConnection.createSession();

    addQueue(address, options);
  } catch(const qpid::types::Exception &ex) {
    THROW(MessageBusException, ex.what());
  }

 
  FromBus::~FromBus(void)
  {
    if (itsNrMissingACKs) {
      LOG_ERROR_STR("[FromBus] " << itsNrMissingACKs << " messages not ACK'ed");
    }

    try {
      // Make sure all requests are finished

      // connection.close() closes all senders, receivers, and sessions as well.
      itsConnection.close();
    } catch(const qpid::types::Exception &ex) {
      LOG_FATAL_STR("Exception in destructor, cannot guarantee message delivery: " << ex.what());
    }
  }

  bool FromBus::getMessage(LOFAR::Message &msg, double timeout) // timeout 0.0 means blocking
  {
    Receiver next;
    qpid::messaging::Message qmsg;

    LOG_DEBUG_STR("[FromBus] Waiting for message");
    if (itsSession.nextReceiver(next,TimeOutDuration(timeout))) {
        LOG_DEBUG_STR("[FromBus] Message available on queue " << next.getName());
        itsNrMissingACKs++;
        if (next.get(qmsg)) {
            msg = LOFAR::Message(qmsg);
            LOG_DEBUG_STR("[FromBus] Message received on queue " << next.getName() << ": " << msg.short_desc());
            return true;
        } else {
          LOG_ERROR_STR("[FromBus] Could not retrieve available message on queue " << next.getName());
        }
    }
    return false;
  }

  void FromBus::ack(LOFAR::Message &msg)
  {
     itsSession.acknowledge(msg.qpidMsg());
     itsNrMissingACKs --;

     LOG_INFO_STR("[FromBus] Message ACK'ed: " << msg.short_desc());
  }

  void FromBus::nack(LOFAR::Message &msg)
  {
     itsSession.release(msg.qpidMsg());
     itsNrMissingACKs--;

     LOG_ERROR_STR("[FromBus] Message NACK'ed: " << msg.short_desc());
  }

  void FromBus::reject(LOFAR::Message &msg)
  {
     itsSession.reject(msg.qpidMsg());
     itsNrMissingACKs --;

     LOG_ERROR_STR("[FromBus] Message rejected: " << msg.short_desc());
  }

  bool FromBus::addQueue(const std::string &address, const std::string &options)
  {
     try
     {
        Address addr(queue_prefix()+address+options);
        Receiver receiver = itsSession.createReceiver(addr);
        receiver.setCapacity(1);

        LOG_INFO_STR("[FromBus] Receiver started at queue " << receiver.getName());
     } catch(const qpid::types::Exception &ex) {
       //THROW(MessageBusException, ex.what());
       return false;
     }
     return true;
  }
} // namespace LOFAR

