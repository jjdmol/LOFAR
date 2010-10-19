//#  ReaderThread.cc: The thread that read from a TH and places data into the buffer of the input section
//#
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <lofar_config.h>

// General includes
#include <Common/LofarLogger.h>
#include <Common/Timer.h>
#include <Transport/TransportHolder.h>

#include <TFC_InputSection/ReaderThread.h>

namespace LOFAR {

  void printTimers(vector<NSTimer*>& timers)
  {
    vector<NSTimer*>::iterator it = timers.begin();
    for (; it != timers.end(); it++) {
      (*it)->print(cout);
    }
  }

  void* ReaderThread(void* arguments)
  {
    LOG_TRACE_FLOW_STR("WH_RSPInput WriterThread");   

    ThreadArgs* args = (ThreadArgs*)arguments;
    
    int seqid = 0;
    int blockid = 0;
    timestamp_t actualstamp;

#define PACKET_STATISTICS_NOT
#ifdef PACKET_STATISTICS
    timestamp_t expectedstamp;
    vector<PacketStats> missedStamps;
    vector<PacketStats> oldStamps;
    missedStamps.reserve(500);
    oldStamps.reserve(500);
#endif
    bool firstloop = true;
    
    // buffer for incoming rsp data
    char totRecvframe[args->PayloadSize + args->IPHeaderSize];
    char* recvframe = totRecvframe;
    int recvDataSize = args->PayloadSize;
    //if (args->Connection->getType() == "TH_Ethernet") {
      recvframe += args->IPHeaderSize;
      recvDataSize = args->PayloadSize + args->IPHeaderSize;
      //};
    
    vector<NSTimer*> itsTimers;
    NSTimer threadTimer("threadTimer");
    NSTimer receiveTimer("receiveTimer");
    NSTimer writeTimer("writeTimer");
    itsTimers.push_back(&threadTimer);
    itsTimers.push_back(&receiveTimer);
    itsTimers.push_back(&writeTimer);


    // init Transportholder
    ASSERTSTR(args->Connection->init(), "Could not init TransportHolder");

    int strideSize = args->EPAPacketSize / sizeof(SubbandType);

    while(!args->Stopthread) {
      threadTimer.start();

      try {
	receiveTimer.start();
	args->Connection->recvBlocking( (void*)totRecvframe, recvDataSize, 0);
	receiveTimer.stop();
      } catch (Exception& e) {
	LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't read from TransportHolder, stopping thread");
	pthread_exit(NULL);
      }	
      // get the actual timestamp of first EPApacket in frame
      seqid   = ((int*)&recvframe[8])[0];
      blockid = ((int*)&recvframe[12])[0];
      actualstamp.setStamp(seqid ,blockid);
      if (args->Connection->getType() == "TH_Null") {
	if (!firstloop) {
	  actualstamp += args->nrPacketsInFrame; 
	} else {
	  actualstamp = timestamp_t(0, 0);
#ifdef PACKET_STATISTICS
	  expectedstamp = actualstamp;
#endif
	  firstloop = false;
	}	  
#ifdef PACKET_STATISTICS
      } else {
	// firstloop
	if (firstloop) {
	  expectedstamp.setStamp(seqid, blockid); // init expectedstamp
	  firstloop = false;
	}
#endif
      }      
      
      // check and process the incoming data
#ifdef PACKET_STATISTICS
      if (actualstamp < expectedstamp) {
	PacketStats rewritten = {actualstamp, expectedstamp};
	oldStamps.push_back(rewritten);
      } else if (actualstamp > expectedstamp) {
	do {
	  PacketStats missed = {actualstamp, expectedstamp};
	  missedStamps.push_back(missed);
	  // increase the expectedstamp
	  expectedstamp += args->nrPacketsInFrame;
	} while (actualstamp > expectedstamp);
      }
      // increase the expectedstamp
      expectedstamp += args->nrPacketsInFrame; 
#endif
      writeTimer.start();
      // expected packet received so write data into corresponding buffer
      args->BBuffer->writeElements((SubbandType*)&recvframe[args->EPAHeaderSize], actualstamp, args->nrPacketsInFrame, strideSize);
      writeTimer.stop();
      threadTimer.stop();
    }

    printTimers(itsTimers);
#ifdef PACKET_STATISTICS
    LOG_INFO("Timestamps of missed packets:");
    vector<PacketStats>::iterator it = missedStamps.begin();
    for (; it != missedStamps.end(); it++) {
      LOG_INFO_STR("MIS " << it->expectedStamp << " missed at time " << it->receivedStamp);
    }
    LOG_INFO_STR("Rewritten packets:");
    vector<PacketStats>::iterator rit = oldStamps.begin();
    for (; rit != oldStamps.end(); rit++) {
      LOG_INFO_STR("REW " << rit->receivedStamp<<" received at time "<< rit->expectedStamp);
    }
#endif
  
    pthread_exit(NULL);
  }
  
}
