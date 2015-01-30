//# MsgBus.h: Wrapper for QPID clients to send and receive AMQP messages.
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

#ifndef LOFAR_MESSAGEBUS_MSGBUS_H
#define LOFAR_MESSAGEBUS_MSGBUS_H

#ifdef HAVE_QPID
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>
#include <qpid/messaging/Address.h>
#endif

#include <Common/Exception.h>

#include <map>

namespace LOFAR {

EXCEPTION_CLASS(MessageBusException, LOFAR::Exception);

class FromBus
{
  const std::string itsBrokerName;
  const std::string itsQueueName;

#ifdef HAVE_QPID
  qpid::messaging::Connection itsConnection;
  qpid::messaging::Session itsSession;
  qpid::messaging::Receiver receiver;
#endif

  int itsNrMissingACKs;

public:
  FromBus(const std::string &address="testqueue" , const std::string &options="; {create: always}", const std::string &broker = "amqp:tcp:127.0.0.1:5672") ;
  ~FromBus(void);

  bool getString(std::string &str,double timeout = 0.0); // timeout 0.0 means blocking

#ifdef HAVE_QPID
  bool getMessage(qpid::messaging::Message &msg, double timeout = 0.0); // timeout 0.0 means blocking
#endif

  void ack(void);
};

class ToBus
{
  const std::string itsBrokerName;
  const std::string itsQueueName;

#ifdef HAVE_QPID
  qpid::messaging::Connection itsConnection;
  qpid::messaging::Session itsSession;
  qpid::messaging::Sender sender;
#endif
  
public:
  ToBus(const std::string &address="testqueue" , const std::string &options="; {create: always}", const std::string &broker = "amqp:tcp:127.0.0.1:5672") ;
  ~ToBus(void);

  void send(const std::string &msg);
};

class MultiBus
{
public:
  typedef bool (* MsgHandler)(const std::string &, const std::string &);

private:
  const std::string itsBrokerName;

  typedef struct
  {
    MsgHandler handler;
    std::string queuename;
  } MsgWorker;

#ifdef HAVE_QPID
  std::map<qpid::messaging::Receiver, MsgWorker> itsHandlers;

  qpid::messaging::Connection itsConnection;
  qpid::messaging::Session itsSession;
#endif

  int itsNrMissingACKs;

public:
  MultiBus(const std::string &broker = "amqp:tcp:127.0.0.1:5672");
  ~MultiBus();

  void addQueue(MsgHandler handler, const std::string &address="testqueue", const std::string &options="; {create: always}");
  void handleMessages(void);
   
#ifdef HAVE_QPID
  bool getMessage(qpid::messaging::Message &msg, double timeout = 0.0); // timeout 0.0 means blocking
#endif

  bool getString(std::string &str, double timeout = 0.0); // timeout 0.0 means blocking

};

} // namespace LOFAR

#endif

