//# EventMessage.h: Message class used for event messages.
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
//# $Id: EventMessage.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_EVENTMESSAGE_H
#define LOFAR_MESSAGING_EVENTMESSAGE_H

// @file
// Message class used for event messages.

#include <Messaging/LofarMessage.h>
#include <string>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    // Message class used for event messages. Events are messages that @e must
    // be delivered. If the message cannot be delivered to the recipient, it
    // will be stored in a persistent queue for later delivery.
    class EventMessage : public LofarMessage
    {
    public:
      // Default constructor.
      EventMessage();

    private:
      // Construct a message from a Qpid message. This constructor is used
      // by the ObjectFactory.
      EventMessage(const qpid::messaging::Message& qmsg);

      // The ObjectFactory needs to have access to the private constructor.
      friend class LOFAR::ObjectFactory< 
        Message*(const qpid::messaging::Message& msg), std::string >; 
    };

    // @}
  }
}

#endif
