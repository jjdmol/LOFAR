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
    
    int seqid, blockid;
    timestamp_t actualstamp, expectedstamp;
    vector<timestamp_t> missedStamps;
    vector<RewriteStats> oldStamps;
    missedStamps.reserve(500);
    oldStamps.reserve(500);
    bool readnew = true;
    bool firstloop = true;
    
    // buffer for incoming rsp data
    char recvframe[args->FrameSize];
    
    // define a block of dummy subband data
    SubbandType dummyblock[args->nrPacketsInFrame];
    memset(dummyblock, 0, args->nrPacketsInFrame*sizeof(SubbandType));
    
    vector<NSTimer*> itsTimers;
    NSTimer threadTimer("threadTimer");
    NSTimer receiveTimer("receiveTimer");
    NSTimer writeTimer("writeTimer");
    NSTimer rewriteTimer("rewriteTimer");
    NSTimer writeDummyTimer("writeDummyTimer");
    itsTimers.push_back(&threadTimer);
    itsTimers.push_back(&receiveTimer);
    itsTimers.push_back(&writeTimer);
    itsTimers.push_back(&rewriteTimer);
    itsTimers.push_back(&writeDummyTimer);

    //  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    //  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    //pthread_cleanup_push(&cleanupWriteBuffer, &itsTimers);

    // init Transportholder
    ASSERTSTR(args->Connection->init(), "Could not init TransportHolder");

    int strideSize = args->EPAPacketSize / sizeof(SubbandType);

    while(!args->Stopthread) {
      threadTimer.start();

      // check stop condition
      if (args->Stopthread == true) {
	pthread_exit(NULL);
    }
      // catch a frame from input connection
      if (readnew) {
	try {
	  if (args->Connection != 0){
	    receiveTimer.start();
	    args->Connection->recvBlocking( (void*)recvframe, args->FrameSize, 0);
	    receiveTimer.stop();
	  }
	} catch (Exception& e) {
	  LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't read from TransportHolder, stopping thread");
	  pthread_exit(NULL);
	}	
      }
  

      if (args->Connection->getType() != "TH_Null") {
	// get the actual timestamp of first EPApacket in frame
	seqid   = ((int*)&recvframe[8])[0];
	blockid = ((int*)&recvframe[12])[0];
	actualstamp.setStamp(seqid ,blockid);
      } else {
	actualstamp += args->nrPacketsInFrame; 
      }      
  
      // firstloop
      if (firstloop) {
	expectedstamp.setStamp(seqid, blockid); // init expectedstamp
      
	//get stationid
	//*args->StationIDptr =((int*)&recvframe[4])[0];
	if (args->IsMaster) {  // temporary hardcoded statioID's master->0, slave->1 
	  *args->StationIDptr = 0;  
	}
	else {
	  *args->StationIDptr = 1;  
	} //end (temporary hardcoded statioID's)
	firstloop = false;
      }
    
      // check and process the incoming data
      if (actualstamp < expectedstamp) {
	RewriteStats rewritten;
	rewritten.oldStamp = actualstamp;
	rewritten.expectedStamp = expectedstamp;
	rewriteTimer.start();
	/* old packet received 
	   Packet can be saved when its dummy is available in cyclic buffer. 
	   Otherwise this packet will be lost */

	int idx;
	int p=0;
	for (; p<args->nrPacketsInFrame; p++) {
	  idx = (p*args->EPAPacketSize) + args->EPAHeaderSize;
	  if (!args->BBuffer->writeElements((SubbandType*)&recvframe[idx], actualstamp, 1, 1, true)) {
	    break;  
	  }
	  actualstamp++;   
	}
	rewritten.succeeded = p;
	oldStamps.push_back(rewritten);
	
	// read new frame in next loop
	readnew = true;
	// do not increase the expectedstamp
	rewriteTimer.stop();
      } else if (actualstamp > expectedstamp) {
	writeDummyTimer.start();
	missedStamps.push_back(expectedstamp);
	// missed a packet so create dummy for that missing packet
	args->BBuffer->writeElements((SubbandType*)dummyblock, expectedstamp, args->nrPacketsInFrame, 0, false);
	// read same frame again in next loop
	readnew = false;
	// increase the expectedstamp
	expectedstamp += args->nrPacketsInFrame; 
	writeDummyTimer.stop();
      } else {
	writeTimer.start();
	// expected packet received so write data into corresponding buffer
	args->BBuffer->writeElements((SubbandType*)&recvframe[args->EPAHeaderSize], actualstamp, args->nrPacketsInFrame, strideSize, true);
	actualstamp += args->nrPacketsInFrame;
	// read new frame in next loop
	readnew = true;
	// increase the expectedstamp
	expectedstamp += args->nrPacketsInFrame; 
	writeTimer.stop();
      }
      threadTimer.stop();
    }
    //pthread_cleanup_pop(1);
    printTimers(itsTimers);
    cout<<"Timestamps of missed packets:"<<endl;
    vector<timestamp_t>::iterator it = missedStamps.begin();
    for (; it != missedStamps.end(); it++) {
      cout<<"MIS " << (*it)<<endl;
    }
    cout<<"Rewritten packets:"<<endl;
    vector<RewriteStats>::iterator rit = oldStamps.begin();
    for (; rit != oldStamps.end(); rit++) {
      cout<<"REW " << rit->oldStamp<<" "<< rit->expectedStamp<<" "<<rit->succeeded<<endl;
    }
  
    pthread_exit(NULL);
  }
  
}
