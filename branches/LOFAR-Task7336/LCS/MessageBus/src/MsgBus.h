/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */

#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>

using namespace qpid::messaging;
using namespace qpid::types;

using std::stringstream;
using std::string;

class FromBus: private Connection 
{
  string queuename,brokername;
  Session session;
  Receiver receiver;
  int DiffNumAck;
  int state;
  void cleanup(void);

public:
  FromBus( char * address="testqueue" ,char * options="; {create: always}", char * broker = "amqp:tcp:127.0.0.1:5672") ;
   // : Connection(broker);
  std::string & GetStr(void);
  Message GetMsg();
  void Ack(void);
  ~FromBus(void);
};

class ToBus: private Connection 
{
  string queuename,brokername;
  Session session;
  Sender sender;
  int state,DiffNumAck;
  void cleanup(void);
  
public:
  ToBus( char * address="testqueue" ,char * options="; {create: always}", char * broker = "amqp:tcp:127.0.0.1:5672") ;
  //: Connection(broker);

  void Send( std::string & m);
  ~ToBus(void);
};

//typedef int (* MsgHandler)(Message &,std::string &);
typedef int (* MsgHandler)(std::string &,std::string &);

typedef struct
{
  MsgHandler handler;
  std::string queuename;
} MsgWorker;

class MultiBus: private Connection 
{
  std::map<Receiver,MsgWorker*> handlers;
  std::string brokername;
  Session session;
  int state;
  int DiffNumAck;
  void cleanup(void);

public:
  MultiBus( MsgHandler handler, char * address="testqueue" ,char *options="; {create: always}", char * broker = "amqp:tcp:127.0.0.1:5672") ;
  //: Connection(broker);
  void add(MsgHandler handler, char * address="testqueue", char*options="; {create: always}");
  void HandleMessages(void);
  Message Get(double timeout =0.0); // timeout 0.0 means blocking
  ~MultiBus();
};



