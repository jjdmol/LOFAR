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


#include "brokermgt.h"
#include <sstream>
#include <string>
#include <iomanip>

#include <qpid/messaging/exceptions.h>
using namespace std;



void brokermgt::list(std::string type)
{
            Message request;
            request.setReplyTo(recv.getAddress());
            request.setProperty("x-amqp-0-10.app-id", "qmf2");
            request.setProperty("qmf.opcode", "_query_request");
            Variant::Map schemaId;
            schemaId["_class_name"] = type;
            Variant::Map content;
            content["_what"] = "OBJECT";
            content["_schema_id"] = schemaId;
            request.setContentObject(content);
	    lasttype=type;
            send.send(request);
}

std::string brokermgt::reply(unsigned long long timeout)
{
      if (lasttype=="queue")
      {
            std::ostringstream ss;

	    Duration tmout(timeout);

	    try {

            Message response = recv.fetch();
            Variant::List contentIn = response.getContentObject().asList();

	    // create a JSON string from the items we need.
	    bool moreitems=false;
            for (Variant::List::const_iterator i = contentIn.begin(); i != contentIn.end(); ++i) {
                Variant::Map item = i->asMap();
                Variant::Map properties = item["_values"].asMap();
                if (properties.find("name") != properties.end()) {
		   if (moreitems) ss << ",";
		   moreitems=true;
		   ss   << '"' <<  properties["name"] 
		        << "\": [" << properties["msgDepth"]
			<< "," << properties["msgTotalDequeues"] 
			<< "," << properties["consumerCount"] << "]" ;
                } else {
                    ss << properties << std::endl;
                }
            }
            sess.acknowledge();
	    return ss.str();
	    } catch (const qpid::messaging::NoMessageAvailable &e)
	    {
		std::string noret("None");
		return noret;
	    };

      }
      return "Unknown";
}

