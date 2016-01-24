//# Message.h: Top-level message class.
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
//# $Id: Message.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_MESSAGE_H
#define LOFAR_MESSAGING_MESSAGE_H

// @file
// Top-level message class.

#include <Common/ObjectFactory.h>
#include <Common/Singleton.h>
#include <qpid/messaging/Message.h>
#include <qpid/types/Variant.h>
#include <set>
#include <string>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    // Top-level message class. 
    // This class publicly inherits from the Qpid Message class, hence it can be
    // used as a Qpid Message anywhere you like.
    class Message : public qpid::messaging::Message
    {
    public:
      // Create a Message object from a Qpid message. The Qpid message property
      // @a MessageType is used to determine which Message object must be
      // constructed.
      // @throw UnknownMessageType
      static Message* create(const qpid::messaging::Message& qmsg);

      // Retrieve a single property directly from a Qpid message.
      // @throw MessagePropertyNotFound
      static qpid::types::Variant
      getProperty(const std::string& property,
                  const qpid::messaging::Message& qmsg);

      // Virtual destructor, because you can inherit from this class.
      virtual ~Message() = 0;

      // Retrieve a single message property.
      // @throw MessagePropertyNotFound
      qpid::types::Variant getProperty(const std::string& property) const;

      // Return the message type as a string
      std::string type() const;

    protected:
      // Default constructor. Creates an empty message object.
      Message();

      // Construct a Message object from a Qpid message.
      Message(const qpid::messaging::Message& msg);

      // // Return a set of property names, i.e. the keys of the properties map.
      // std::set<std::string> propertyNames() const;
    };

    // Generic factory for Message objects.
    typedef LOFAR::Singleton<
      LOFAR::ObjectFactory< 
        Message*(const qpid::messaging::Message& msg), std::string > 
      > MessageFactory;

    // @}

  }
}

#endif
