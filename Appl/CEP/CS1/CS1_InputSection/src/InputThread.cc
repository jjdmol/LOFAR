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
#include <CS1_InputSection/InputThread.h>
#include <Common/Timer.h>
#include <Transport/TransportHolder.h>
#include <CS1_InputSection/BeamletBuffer.h>

namespace LOFAR {
  namespace CS1_InputSection {

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
      missedStamps.reserve(500);
      oldStamps.reserve(500);
#endif

      bool firstloop = true;
    
      // buffer for incoming rsp data
      int frameSize = itsArgs.frameSize;
      // reserve space in case there is an ip header in front of the packet
      char totRecvframe[frameSize + itsArgs.ipHeaderSize];
      char* recvframe = totRecvframe;
      //      if (itsArgs.th->getType() == "TH_Ethernet") {
	// only with TH_Ethernet there is an IPHeader
        // but also when we have recorded it from the rsp boards!
	recvframe += itsArgs.ipHeaderSize;
	frameSize += itsArgs.ipHeaderSize;
	//      };
    
      vector<NSTimer*> itsTimers;
      NSTimer threadTimer("threadTimer");
      NSTimer receiveTimer("receiveTimer");
      NSTimer writeTimer("writeTimer");
      itsTimers.push_back(&threadTimer);
      itsTimers.push_back(&receiveTimer);
      itsTimers.push_back(&writeTimer);

      // init Transportholder
      ASSERTSTR(itsArgs.th->init(), "Could not init TransportHolder");

      // how far is one beamlet of a subband away from the next beamlet of the same subband
      int strideSize = itsArgs.packetSize / sizeof(Beamlet);

      while(!theirShouldStop) {
	threadTimer.start();

	try {
	  receiveTimer.start();
	  itsArgs.th->recvBlocking( (void*)totRecvframe, frameSize, 0);
	  receiveTimer.stop();
	} catch (Exception& e) {
	  LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't read from TransportHolder("<<e.what()<<", stopping thread");
	  break;
	}	

	// get the actual timestamp of first EPApacket in frame
	seqid   = ((int*)&recvframe[8])[0];
	blockid = ((int*)&recvframe[12])[0];
	actualstamp.setStamp(seqid ,blockid);
	if (itsArgs.th->getType() == "TH_Null") {
	  if (!firstloop) {
	    actualstamp += itsArgs.nPacketsPerFrame; 
	  } else {
	    actualstamp = TimeStamp(0, 0);
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
	    expectedstamp += itsArgs.nPacketsPerFrame;
	  } while (actualstamp > expectedstamp);
	}
	// increase the expectedstamp
	expectedstamp += itsArgs.nPacketsPerFrame; 
#endif
	writeTimer.start();
	// expected packet received so write data into corresponding buffer
	itsArgs.BBuffer->writeElements((Beamlet*)&recvframe[itsArgs.frameHeaderSize], actualstamp, itsArgs.nPacketsPerFrame, strideSize);
	writeTimer.stop();
	threadTimer.stop();
      }

      printTimers(itsTimers);
#ifdef PACKET_STATISTICS
      LOG_TRACE_INFO("Timestamps of missed packets:");
      vector<PacketStats>::iterator it = missedStamps.begin();
      for (; it != missedStamps.end(); it++) {
	LOG_TRACE_INFO_STR("MIS " << it->expectedStamp << " missed at time " << it->receivedStamp);
      }
      LOG_TRACE_INFO_STR("Rewritten packets:");
      vector<PacketStats>::iterator rit = oldStamps.begin();
      for (; rit != oldStamps.end(); rit++) {
	LOG_TRACE_INFO_STR("REW " << rit->receivedStamp<<" received at time "<< rit->expectedStamp);
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

  } // namespace CS1_InputSection
} // namespace LOFAR
