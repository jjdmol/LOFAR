//# FromBus.cc: Provide an easy way to fetch messages from the message bus.
//#
//# Copyright (C) 2015
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR Software Suite.
//#
//# The LOFAR Software Suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published by
//# the Free Software Foundation, either version 3 of the License, or (at your
//# option) any later version.
//#
//# The LOFAR Software Suite is distributed in the hope that it will be
//# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along with
//# The LOFAR Software Suite.  If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: FromBus.cc 1483 2015-08-23 19:19:44Z loose $

#include <lofar_config.h>

#include "Helpers.h"

#include <Messaging/Exceptions.h>
#include <Messaging/FromBus.h>
#include <Messaging/Message.h>
#include <Common/LofarLogger.h>

#include <qpid/messaging/Receiver.h>
#include <qpid/types/Exception.h>

namespace LOFAR
{
  namespace Messaging
  {
    using namespace std;
    using qpid::messaging::Receiver;

    FromBus::FromBus()
    {
    }

    FromBus::FromBus(const std::string& address, 
                     const std::string& options,
                     const std::string& broker)
    // We need to use a function try-block here, because we want to catch
    // exceptions that may be thrown during member initialization as well.
    try :
      itsConnection(broker, defaultBrokerOptions),
      itsNrMissingACKs(0)
      {
        LOG_DEBUG_STR("[FromBus] Connecting to broker: " << broker);
        itsConnection.open();
        LOG_INFO_STR("[FromBus] Connected to broker: " << itsConnection.getUrl());
        itsSession = itsConnection.createSession();
        addQueue(address, options);
      } 
    catch (qpid::types::Exception& ex) {
      THROW (MessagingException, ex.what());
    }


    FromBus::~FromBus()
    {
      if (itsNrMissingACKs) {
        LOG_ERROR_STR(
          "[FromBus] " << itsNrMissingACKs << " message" << 
          (itsNrMissingACKs == 1 ? "" : "s") << " not acknowledged");
      }
      string url(itsConnection.getUrl());
      LOG_DEBUG_STR("[FromBus] Closing connection: " << url);
      itsConnection.close();
      LOG_INFO_STR("[FromBus] Closed connection: " << url);
    }


    void FromBus::addQueue(const std::string& address, 
                           const std::string& options)
    {
      try {
        string fullAddr(address + (options.empty() ? "" : "; " + options));
        LOG_DEBUG_STR("[FromBus] Creating receiver: " << fullAddr);
        Receiver receiver = itsSession.createReceiver(fullAddr);
        receiver.setCapacity(defaultReceiverCapacity);
        LOG_INFO_STR("[FromBus] Receiver created at: " << receiver.getName());
      }
      catch (qpid::types::Exception& ex) {
        THROW (MessagingException, ex.what());
      }
    }

    Message* FromBus::getMessage(double timeout)
    {
      Receiver next;
      qpid::messaging::Message qmsg;

      LOG_DEBUG_STR("[FromBus] Waiting for message");
      if (!itsSession.nextReceiver(next,TimeOutDuration(timeout))) {
        LOG_DEBUG_STR("[FromBus] Time-out while waiting for message");
        return 0;
      }
      LOG_DEBUG_STR("[FromBus] Message available on queue " << next.getName());
      itsNrMissingACKs++;
      if (next.get(qmsg)) {
        Message* message = Message::create(qmsg);
        LOG_INFO_STR("[FromBus] Received " << message->type()
                     << " on queue " << next.getName());
        return message;
      } else {
        LOG_ERROR_STR("[FromBus] Could not retrieve available message "
                      "on queue " << next.getName());
        return 0;
      }
    }

    void FromBus::ack(Message& message) 
    {
      itsSession.acknowledge(message);
      itsNrMissingACKs--;
    }

    void FromBus::nack(Message& message)
    {
      itsSession.release(message);
      itsNrMissingACKs--;
    }

    void FromBus::reject(Message& message)
    {
      itsSession.reject(message);
    }

  }
}
