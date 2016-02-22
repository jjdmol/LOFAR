//# FromBus.h: Wrapper for QPID clients to receive AMQP messages.
//#
//# Copyright (C) 2015
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_MESSAGEBUS_FROMBUS_H
#define LOFAR_MESSAGEBUS_FROMBUS_H

#ifdef HAVE_QPID
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Session.h>
#include <qpid/messaging/Address.h>
#else
#include <MessageBus/NoQpidFallback.h>
#endif

#include <Common/Exception.h>
#include <MessageBus/Message.h>

#include <map>
#include <string>

namespace LOFAR {


class FromBus
{
public:
  FromBus(const std::string &address, const std::string &options="; {create: never}", const std::string &broker = "amqp:tcp:127.0.0.1:5672") ;
  ~FromBus(void);
  bool addQueue(const std::string &address, const std::string &options="; {create: never}");

  bool getMessage(LOFAR::Message &msg, double timeout = 0.0); // timeout 0.0 means blocking

  void ack   (LOFAR::Message &msg);
  void nack  (LOFAR::Message &msg);
  void reject(LOFAR::Message &msg);

private:
  qpid::messaging::Connection itsConnection;
  qpid::messaging::Session    itsSession;

  int itsNrMissingACKs;
};

} // namespace LOFAR

#endif

