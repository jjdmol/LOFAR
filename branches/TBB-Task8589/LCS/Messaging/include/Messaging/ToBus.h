//# ToBus.h: Provide an easy way to put messages onto the message bus.
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
//# $Id: ToBus.h 1483 2015-08-23 19:19:44Z loose $

#ifndef LOFAR_MESSAGING_TOBUS_H
#define LOFAR_MESSAGING_TOBUS_H

// @file ToBus.h
// Provide an easy way to put messages onto the message bus.

#include <Messaging/DefaultSettings.h>
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Session.h>
#include <qpid/messaging/Sender.h>
#include <string>

namespace LOFAR
{
  namespace Messaging
  {
    // @addtogroup Messaging
    // @{

    // This class provides an easy way to put messages onto the message bus.
    class ToBus
    {
    public:
      // Constructor.
      // @param address valid Qpid address.
      // @param options valid Qpid address options.
      // @param broker  valid Qpid broker URL.
      // @note  Please consult the Qpid documentation for more details.
      ToBus(const std::string& address, 
            const std::string& options = defaultAddressOptions,
            const std::string& broker = defaultBroker);

      // Destructor.
      ~ToBus();

      // Put the given message onto the bus.
      void send(const Message& message);

    private:
      // Add a queue that you want to put messages into. 
      void addQueue(const std::string& address, 
                    const std::string& options);

      qpid::messaging::Connection itsConnection;
      qpid::messaging::Session itsSession;
      qpid::messaging::Sender itsSender;
    };

    // @}
  }
}

#endif
