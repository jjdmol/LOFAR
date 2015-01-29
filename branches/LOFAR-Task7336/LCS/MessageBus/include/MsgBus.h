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

#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>

class FromBus: private qpid::messaging::Connection 
{
  std::string queuename,brokername;
  qpid::messaging::Session session;
  qpid::messaging::Receiver receiver;
  int DiffNumAck;
  int state;
  void cleanup(void);

public:
  FromBus(const std::string &address="testqueue" , const std::string &options="; {create: always}", const std::string &broker = "amqp:tcp:127.0.0.1:5672") ;
  std::string GetStr(double timeout =0.0); // timeout 0.0 means blocking
  qpid::messaging::Message GetMsg(double timeout =0.0); // timeout 0.0 means blocking
  void Ack(void);
  ~FromBus(void);
};

class ToBus: private qpid::messaging::Connection 
{
  std::string queuename,brokername;
  qpid::messaging::Session session;
  qpid::messaging::Sender sender;
  int state,DiffNumAck;
  void cleanup(void);
  
public:
  ToBus(const std::string &address="testqueue" , const std::string &options="; {create: always}", const std::string &broker = "amqp:tcp:127.0.0.1:5672") ;

  void Send( std::string & m);
  ~ToBus(void);
};

typedef int (* MsgHandler)(std::string &,std::string &);

typedef struct
{
  MsgHandler handler;
  std::string queuename;
} MsgWorker;

class MultiBus: private qpid::messaging::Connection 
{
  std::map<qpid::messaging::Receiver, MsgWorker*> handlers;
  std::string brokername;
  qpid::messaging::Session session;
  int state;
  int DiffNumAck;
  void cleanup(void);

public:
  MultiBus(MsgHandler handler, const std::string &address="testqueue", const std::string &options="; {create: always}", const std::string &broker = "amqp:tcp:127.0.0.1:5672") ;
  void add(MsgHandler handler, const std::string &address="testqueue", const std::string &options="; {create: always}");
  void HandleMessages(void);
  qpid::messaging::Message Get(double timeout =0.0); // timeout 0.0 means blocking
  ~MultiBus();
};

#endif

