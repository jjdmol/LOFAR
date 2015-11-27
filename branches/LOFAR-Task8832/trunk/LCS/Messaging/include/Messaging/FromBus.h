//# FromBus.h: Provide an easy way to fetch messages from the message bus.
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
//# $Id: FromBus.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_FROMBUS_H
#define LOFAR_MESSAGING_FROMBUS_H

// @file
// Provide an easy way to fetch messages from the message bus.

#include <Messaging/DefaultSettings.h>
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Session.h>
#include <string>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    //# Forward declarations
    class Message;

    // This class provides an easy way to fetch messages from the message bus.
    class FromBus
    {
    public:
      // Default constructor.
      FromBus();

      // Constructor.
      // @param address valid Qpid address.
      // @param options valid Qpid address options.
      // @param broker  valid Qpid broker URL.
      // @note  Please consult the Qpid documentation for more details.
      FromBus(const std::string& address, 
              const std::string& options = defaultAddressOptions,
              const std::string& broker = defaultBroker);

      // Destructor. Report the number of missing acknowledgements if non-zero.
      ~FromBus();

      // Add a queue that you want to receive messages from.
      // @param address valid Qpid address
      // @param options valid Qpid address options.
      void addQueue(const std::string& address, 
                    const std::string& options = defaultAddressOptions);

      // Retrieve the next message from any of the queues we're listening on.
      // @param timeout Maximum time in seconds to wait for a message.
      // @return Pointer to a new Message object or a null pointer if no message
      // arrived within @a timeout seconds. The user is responsible for deleting
      // the object.
      Message* getMessage(double timeout = defaultTimeOut);

      // Acknowledge a message. This will inform Qpid that the message can
      // safely be removed from the queue.
      // @param message Message to acknowledge.
      void ack(Message& message);

      // Do not acknowledge a message. This will inform Qpid that the message
      // has to be redelivered. You cannot nack a message that has already
      // been acknowledged.
      // @param message Message to not be acknowledged.
      void nack(Message& message);

      // Reject a message. This will inform Qpid that the message should not be
      // redelivered. You cannot reject a message that has already been
      // acknowledged.
      // @param message Message to be rejected.
      void reject(Message& message);

    private:
      qpid::messaging::Connection itsConnection;
      qpid::messaging::Session itsSession;

      // Keep track of the number of missing acknowledgements.
      unsigned itsNrMissingACKs;
    };

    // @}

  }
}

#endif
