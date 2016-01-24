//# Message.cc: Top-level message class.
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
//# $Id: Message.cc 1483 2015-08-23 19:19:44Z loose $

#include <lofar_config.h>
#include <Messaging/Message.h>
#include <Messaging/Exceptions.h>

namespace LOFAR
{
  namespace Messaging
  {
    using namespace std;

    //## ----  Definition of public methods  ---- ##//

    Message* Message::create(const qpid::messaging::Message& qmsg)
    {
      string msgType(getProperty("MessageType", qmsg));
      Message* msg = MessageFactory::instance().create(msgType, qmsg);
      if (!msg) {
        THROW (UnknownMessageType,
               "Don't know how to create a message of type " << msgType <<
               ". Did you register the class with the factory?");
      }
      return msg;
    }


    qpid::types::Variant
    Message::getProperty(const string& property,
                         const qpid::messaging::Message& qmsg)
    {
      const qpid::types::Variant::Map& properties(qmsg.getProperties());
      qpid::types::Variant::Map::const_iterator it;
      it = properties.find(property);
      if (it == properties.end()) {
        THROW (MessagePropertyNotFound,
               "Message property '" << property << "' not found");
      }
      return it->second;
    }


    Message::~Message()
    {
    }


    string Message::type() const
    {
      return getProperty("MessageType");
    }


    //## ----  Definition of protected methods  ---- ##//

    Message::Message() :
      qpid::messaging::Message()
    {
    }


    Message::Message(const qpid::messaging::Message& qmsg) :
      qpid::messaging::Message(qmsg)
    {
    }


    qpid::types::Variant Message::getProperty(const std::string& property) const
    {
      return getProperty(property, *this);
    }


    // set<string> Message::propertyNames() const
    // {
    //   set<string> keys;
    //   const qpid::types::Variant::Map& properties(getProperties());
    //   qpid::types::Variant::Map::const_iterator it;
    //   for(it = properties.begin(); it != properties.end(); ++it) {
    //     keys.insert(it->first);
    //   }
    //   return keys;
    // }

  }
}
