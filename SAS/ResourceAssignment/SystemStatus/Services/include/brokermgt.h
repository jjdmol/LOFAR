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


#include <qpid/messaging/Address.h>
#include <qpid/messaging/Connection.h>
#include <qpid/messaging/Message.h>
#include <qpid/messaging/Receiver.h>
#include <qpid/messaging/Sender.h>
#include <qpid/messaging/Session.h>
#include <qpid/types/Variant.h>
#include <iostream>

using namespace qpid::messaging;
using namespace qpid::types;

#include <string>

static const qpid::types::Variant::Map connopts={{ "reconnect",true }};

class brokermgt
{
    private:
	std::string broker;
	Connection conn;
	Session sess;	
       	Receiver recv;
	Sender send;
	std::string lasttype;

    public:
	brokermgt(std::string host) : broker(host),conn(host,connopts),sess((conn.open(),conn.createSession())),
				      recv(sess.createReceiver("#")),
	 			      send(sess.createSender("qmf.default.direct/broker"))
       {
       };

       //void docmd(std::string command,std::string type);
       void list(std::string type);// { docmd("list",type);};
       std::string reply(unsigned long long timeout=0);

       ~brokermgt()
       {
           send.close();
           conn.close();
       };
       const std::string brokername()
       {
	   return broker;
       }
};

