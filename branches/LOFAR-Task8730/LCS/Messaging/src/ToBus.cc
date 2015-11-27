//# ToBus.cc: Provide an easy way to put messages onto the message bus.
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
//# $Id: ToBus.cc 1483 2015-08-23 19:19:44Z loose $

#include <lofar_config.h>

#include <Messaging/Exceptions.h>
#include <Messaging/Message.h>
#include <Messaging/ToBus.h>
#include <Common/LofarLogger.h>

#include <qpid/types/Exception.h>

namespace LOFAR
{
  namespace Messaging
  {
    using namespace std;

    ToBus::ToBus(const string& address, 
                 const string& options,
                 const string& broker)
    // We need to use a function try-block here, because we want to catch
    // exceptions that may be thrown during member initialization as well.
    try : 
      itsConnection(broker, defaultBrokerOptions)
    {
      LOG_DEBUG_STR("[ToBus] Connecting to broker: " << broker);
      itsConnection.open();
      LOG_INFO_STR("[ToBus] Connected to broker: " << itsConnection.getUrl());
      itsSession = itsConnection.createSession();
      addQueue(address, options);
    }
    catch (qpid::types::Exception& ex) {
      THROW (MessagingException, ex.what());
    }


    ToBus::~ToBus()
    {
      string url(itsConnection.getUrl());
      LOG_DEBUG_STR("[ToBus] Closing connection: " << url);
      itsConnection.close();
      LOG_INFO_STR("[ToBus] Closed connection: " << url);
    }


    void ToBus::send(const Message& message)
    {
      LOG_DEBUG_STR("[ToBus] Sending message to queue " << itsSender.getName());
      itsSender.send(message, /*sync*/ true);
      LOG_INFO_STR("[ToBus] Sent message to queue " << itsSender.getName());
    }


    void ToBus::addQueue(const string& address, 
                         const string& options)
    {
      string fullAddr(address + (options.empty() ? "" : "; " + options));
      LOG_DEBUG_STR("[ToBus] Creating sender: " << fullAddr);
      itsSender = itsSession.createSender(fullAddr);
      LOG_INFO_STR("[ToBus] Sender created at: " << itsSender.getName());
    }


  }
}
