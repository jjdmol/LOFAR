//#  InputThread.cc: the thread that reads from a TH and places data into the buffer of the input section
//#
//#  Copyright (C) 2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/hexdump.h>
#include <CS1_InputSection/InputThread.h>
#include <Common/Timer.h>
#include <Transport/TransportHolder.h>
#include <CS1_InputSection/BeamletBuffer.h>

namespace LOFAR {
  namespace CS1 {

    bool InputThread::theirShouldStop = false;

    InputThread::InputThread(ThreadArgs args) : itsArgs(args)
    {}

    InputThread::~InputThread()
    {}

    void InputThread::operator()()
    {
      LOG_TRACE_FLOW_STR("WH_RSPInput WriterThread");   
      int seqid = 0;
      int blockid = 0;
      TimeStamp actualstamp;

#define PACKET_STATISTICS_NOT
#ifdef PACKET_STATISTICS
      TimeStamp expectedstamp;
      vector<PacketStats> missedStamps;
      vector<PacketStats> oldStamps;
      vector<PacketStats> invalidStamps;
      missedStamps.reserve(500);
      oldStamps.reserve(500);
      invalidStamps.reserve(500);
#endif

      bool firstloop = true;
    
      // buffer for incoming rsp data
      int frameSize = itsArgs.frameSize;
      // reserve space in case there is an ip header in front of the packet
      char totRecvframe[frameSize + itsArgs.ipHeaderSize];
      memset(totRecvframe, 0, sizeof(totRecvframe));
      char* recvframe = totRecvframe;
      if (itsArgs.th->getType() == "TH_Ethernet") {
	// only with TH_Ethernet there is an IPHeader
        // but also when we have recorded it from the rsp boards!
	recvframe += itsArgs.ipHeaderSize;
	frameSize += itsArgs.ipHeaderSize;
      };
    
      vector<NSTimer*> itsTimers;
      NSTimer threadTimer("threadTimer");
      NSTimer receiveTimer("receiveTimer");
      NSTimer writeTimer("writeTimer");
      itsTimers.push_back(&threadTimer);
      itsTimers.push_back(&receiveTimer);
      itsTimers.push_back(&writeTimer);

      // init Transportholder
      ASSERTSTR(itsArgs.th->init(), "Could not init TransportHolder");

      bool dataContainsValidStamp = (itsArgs.th->getType() != "TH_Null");

      while(!theirShouldStop) {
	threadTimer.start();

	bool validTimeStampReceived = false;
	while (!validTimeStampReceived) {
	  try {
	    receiveTimer.start();
	    //cerr<<"InputThread "<<itsArgs.ID << " reading "<<frameSize<<" bytes from TH ("<<(void*)itsArgs.th<<" into "<<(void*)totRecvframe<<endl;
	    itsArgs.th->recvBlocking( (void*)totRecvframe, frameSize, 0);
	    receiveTimer.stop();
	  } catch (Exception& e) {
	    LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't read from TransportHolder("<<e.what()<<", stopping thread");
	    break;
	  }	

	  // get the actual timestamp of first EPApacket in frame
	  if (!dataContainsValidStamp) {
	    if (!firstloop) {
	      actualstamp += itsArgs.nTimesPerFrame; 
	    } else {
	      actualstamp = TimeStamp(0, 0);
#ifdef PACKET_STATISTICS
	      expectedstamp = actualstamp;
#endif
	      firstloop = false;
	    }	  
	    validTimeStampReceived = true;
	  } else {
	    seqid   = ((int*)&recvframe[8])[0];
	    blockid = ((int*)&recvframe[12])[0];
	    validTimeStampReceived = (seqid != 0xffff); //if the second counter has 0xffff, the data cannot be trusted.
	    //cerr<<"InputThread received valid? :"<<validTimeStampReceived<<endl;
#ifdef PACKET_STATISTICS	    
	    if (!validTimeStampReceived) invalidStamps.push_back(PacketStats(actualstamp, expectedstamp));
	    // hack for error in rsp boards where timestamps are not equal on both boards
#endif
	    actualstamp.setStamp(seqid ,blockid);
	    //cerr<<"InputThread received stamp: "<<actualstamp<<" ("<<seqid<<", "<<blockid<<")"<<endl;

	    //cerr<<endl<<"Reading stamp: " << actualstamp<<endl;
#ifdef PACKET_STATISTICS
	    if (firstloop) {
	      // firstloop
	      expectedstamp.setStamp(seqid, blockid); // init expectedstamp
	      firstloop = false;
	    }
#endif
	  }
	}
      
	// check and process the incoming data
#ifdef PACKET_STATISTICS
	if (actualstamp < expectedstamp) {
	  oldStamps.push_back(PacketStats(actualstamp, expectedstamp));
	} else if (actualstamp > expectedstamp) {
	  do {
	    missedStamps.push_back(PacketStats(actualstamp, expectedstamp));
	    // increase the expectedstamp
	    expectedstamp += itsArgs.nTimesPerFrame;
	  } while (actualstamp > expectedstamp);
	}
	// increase the expectedstamp
	expectedstamp += itsArgs.nTimesPerFrame; 
#endif
	writeTimer.start();
	// expected packet received so write data into corresponding buffer
	//cerr<<"InputThread: "<<actualstamp<<endl;

	try {
	  itsArgs.BBuffer->writeElements((Beamlet*)&recvframe[itsArgs.frameHeaderSize], actualstamp, itsArgs.nTimesPerFrame, itsArgs.nSubbandsPerFrame);
	} catch (Exception& e) {
	  LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't write to BeamletBuffer("<<e.what()<<", stopping thread");
	  break;
	}	

	writeTimer.stop();
	threadTimer.stop();
      }

      printTimers(itsTimers);
#ifdef PACKET_STATISTICS
      LOG_WARN("Timestamps of missed packets:");
      vector<PacketStats>::iterator it = missedStamps.begin();
      for (; it != missedStamps.end(); it++) {
	LOG_WARN_STR("MIS " << it->expectedStamp << " missed at time " << it->receivedStamp);
      }
      LOG_WARN_STR("Rewritten packets:");
      vector<PacketStats>::iterator rit = oldStamps.begin();
      for (; rit != oldStamps.end(); rit++) {
	LOG_WARN_STR("REW " << rit->receivedStamp<<" received at time "<< rit->expectedStamp);
      }
      LOG_WARN_STR("Invalid timestamps:");
      vector<PacketStats>::iterator iit = invalidStamps.begin();
      for (; iit != invalidStamps.end(); iit++) {
	LOG_WARN_STR("INV received at time "<< iit->expectedStamp);
      }
#endif
    }

    void InputThread::printTimers(vector<NSTimer*>& timers)
    {
      vector<NSTimer*>::iterator it = timers.begin();
      for (; it != timers.end(); it++) {
	(*it)->print(cout);
      }
    }

    InputThread::InputThread (const InputThread& that)
      : itsArgs(that.itsArgs)
    {}
    ////InputThread& InputThread::operator= (const InputThread& that)
    ////{
    ////  if (this != &that) {
    ////    ... copy members ...
    ////  }
    ////  return *this;
    ////}

  } // namespace CS1
} // namespace LOFAR
