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

#include <string>
#include <map>
#include "brokermgt.h"
#include "broadcast.h"
#include <unistd.h>

#include <thread>

using namespace std;
template <typename T,size_t N> int size(T (&a)[N]){ return N;}

void startbroker(string brokername,brokermgt** dest)
{
    *dest = new brokermgt(brokername);
    cout << "connected to broker at " << brokername << endl;
}

int main(int argc, char** argv)
{
    broadcast Bcast("queuenotify","notifications");
    std::string type = "queue";
    std::string host = "localhost";

    brokermgt * broker[120];
    string brokername[120];
    thread tbroker[120];
    int sbroker[120];


    int numbrokers=0;

    for (numbrokers=1;numbrokers<95;numbrokers++)
    {
	char name[256];
	sprintf(name,"locus%03d.cep2.lofar",numbrokers);
	std::string host(name);
	brokername[numbrokers]=host;
	broker[numbrokers]=0;
	tbroker[numbrokers]=std::thread(startbroker,host,&broker[numbrokers]);
	sbroker[numbrokers]=1;

	//std::cout << "Connecting to " << host << std::endl;
	//broker[numbrokers]=new brokermgt(host);
    }
#ifndef __RUN_ON_PRD__
    std::string brokerlist[] = { "CCU001.control.lofar","sas001.control.lofar","lcs023.control.lofar","cbm001.control.lofar","mcu001.control.lofar","lhn001.cep2.lofar"};
#else
    std::string brokerlist[] ={ "amqp://ccu099","amqp://SAS099","amqp://lcs028","amqp://cbt009","amqp://locus098","amqp://locus099","amqp://locus102","amqp://mcu099"};
#endif
    int num_other=size(brokerlist);

    int t=0;
    while (t<num_other)
    {
	broker[numbrokers]=0;
	brokername[numbrokers]=brokerlist[t];
	cout << " adding broker " << brokername[numbrokers] << endl;
	tbroker[numbrokers]=std::thread(startbroker,brokername[numbrokers],&broker[numbrokers]);
	sbroker[numbrokers]=1;
	numbrokers++;t++;
    }
    /*
    for (t=1;t<numbrokers;t++)
    {

	try {
	std::cout << "Connecting to " << host << std::endl;
	broker[numbrokers]=new brokermgt(brokerlist[t++]);
	}
	catch (qpid::messaging::TransportFailure &e)
	{
	    std::cout << "Connection failed" << std::endl;
	}
    }
    */
    // num_brokers should be the total number of queried brokers.

    while (true)
    {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];

	string output="{";
        for (int i=1;i<numbrokers;i++)
	    if (broker[i]) 
	    {
		// cleanup connector thread
		if (sbroker[i])
		{
		   tbroker[i].join();
		   sbroker[i]=0;
		}
	      broker[i]->list(type);
	    }
	time (&rawtime);
	//timeinfo = localtime(&rawtime);
	timeinfo = gmtime(&rawtime);
	strftime(buffer,80,"%d-%m-%Y %I:%M:%S",timeinfo);
	std::string mytime(buffer);
	output.append("\"datestamp\":\"");
	output.append(mytime);
	output.append("\"");

	for (int i=1;i<numbrokers;i++)
	    if (broker[i])
	    {
	        output.append(",\"");
	        output.append(broker[i]->brokername());
	        output.append("\":{");
	        output.append(broker[i]->reply(1000));
	        output.append("}");
	    }
	output.append("}");
	Bcast(output);
	cout << " sending.." << endl;
	usleep(1000000);
    }

    return 0;
}
