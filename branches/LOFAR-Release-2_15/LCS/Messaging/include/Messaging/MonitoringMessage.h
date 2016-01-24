//# MonitoringMessage.h: Message class used for Monitoring messages.
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
//# $Id: MonitoringMessage.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_MONITORINGMESSAGE_H
#define LOFAR_MESSAGING_MONITORINGMESSAGE_H

// @file
// Message class used for Monitoring messages.

#include <Messaging/LofarMessage.h>
#include <string>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    // Message class used for Monitoring messages. Monitoring messages are
    // publish-subscribe type of messages. They will be not be queued, so they
    // will be lost if there are no subscribers.
    class MonitoringMessage : public LofarMessage
    {
    public:
      // Default constructor.
      MonitoringMessage();

    private:
      // Construct a message from a Qpid message. This constructor is used
      // by the ObjectFactory.
      MonitoringMessage(const qpid::messaging::Message& qmsg);

      // The ObjectFactory needs to have access to the private constructor.
      friend class LOFAR::ObjectFactory< 
        Message*(const qpid::messaging::Message& msg), std::string >; 
    };

    // @}
  }
}

#endif
