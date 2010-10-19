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
#include <Common/DataConvert.h>
#include <Common/Timer.h>
#include <Transport/TransportHolder.h>
#include <BeamletBuffer.h>
#include <InputThread.h>

namespace LOFAR {
namespace CS1 {

volatile bool InputThread::theirShouldStop = false;
volatile unsigned InputThread::nrPacketsReceived, InputThread::nrPacketsRejected;

InputThread::InputThread(const ThreadArgs &args) : itsArgs(args)
{
  std::clog << "InputThread::InputThread(...)" << std::endl;
  if (pthread_create(&thread, 0, mainLoopStub, this) != 0) {
    std::cerr << "could not create input thread" << std::endl;
    exit(1);
  }
}

InputThread::~InputThread()
{
  std::clog << "InputThread::~InputThread()" << std::endl;
  theirShouldStop = true;

  if (pthread_join(thread, 0) != 0) {
    std::cerr << "could not join input thread" << std::endl;
    exit(1);
  }
}

// log from separate thread, since printing from a signal handler causes deadlocks

void *InputThread::logThread(void *)
{
  std::clog << "InputThread::logThread()" << std::endl;
  while (!theirShouldStop) {
#if 0
    static unsigned count;

    if ((++ count & 63) == 0)
      system("cat /proc/meminfo");
#endif

    std::clog <<
	"received " << nrPacketsReceived << " packets, "
	"rejected " << nrPacketsRejected << " packets" << std::endl;
    nrPacketsReceived = nrPacketsRejected = 0; // race conditions, but who cares
    sleep(1);
  }

  std::clog << "InputThread::logThread() finished" << std::endl;
  return 0;
}

void *InputThread::mainLoopStub(void *inputThread)
{
  std::clog << "InputThread::mainLoopStub()" << std::endl;
  reinterpret_cast<InputThread *>(inputThread)->mainLoop();
  return 0;
}


void InputThread::mainLoop()
{
  std::clog << "InputThread::mainLoop()" << std::endl;
  LOG_TRACE_FLOW_STR("WH_RSPInput WriterThread");   

  pthread_t logThreadId;

  if (pthread_create(&logThreadId, 0, logThread, 0) != 0) {
    std::cerr << "could not create log thread " << std::endl;
    exit(1);
  }

  TimeStamp actualstamp = -itsArgs.nTimesPerFrame;

  // buffer for incoming rsp data
  int frameSize = itsArgs.frameSize;
  // reserve space in case there is an ip header in front of the packet
  char totRecvframe[frameSize + itsArgs.ipHeaderSize];
  char *recvframe = totRecvframe;

  if (itsArgs.th->getType() == "TH_Ethernet") {
    // only with TH_Ethernet there is an IPHeader
    // but also when we have recorded it from the rsp boards!
    recvframe += itsArgs.ipHeaderSize;
    frameSize += itsArgs.ipHeaderSize;
  };

  ASSERTSTR(itsArgs.th->init(), "Could not init TransportHolder");

  NSTimer receiveTimer("receiveTimer", true), writeTimer("writeTimer", true);
  bool dataShouldContainValidStamp = (itsArgs.th->getType() != "TH_Null");

  std::clog << "InputThread::mainLoop() entering loop" << std::endl;

  while (!theirShouldStop) {
retry: // until valid packet received

    try {
      receiveTimer.start();
      itsArgs.th->recvBlocking((void *) totRecvframe, frameSize, 0);
      receiveTimer.stop();
      ++ nrPacketsReceived;
    } catch (Exception &e) {
      LOG_TRACE_FLOW_STR("WriteToBufferThread couldn't read from TransportHolder(" << e.what() << ", exiting");
      exit(1);
    }	

    // get the actual timestamp of first EPApacket in frame
    if (dataShouldContainValidStamp) {
      unsigned seqid   = * ((unsigned *) &recvframe[8]);
      unsigned blockid = * ((unsigned *) &recvframe[12]);

#if defined WORDS_BIGENDIAN
      seqid   = byteSwap(seqid);
      blockid = byteSwap(blockid);
#endif
//std::clog << "InputThread::mainLoop: seqid = " << seqid << ", blockid = " << blockid << std::endl;

      //if the seconds counter is 0xFFFFFFFF, the data cannot be trusted.
      if (seqid == ~0U) {
	++ nrPacketsRejected;
	goto retry;
      }

      actualstamp.setStamp(seqid, blockid);
    } else {
      actualstamp += itsArgs.nTimesPerFrame; 
    }
  
    // expected packet received so write data into corresponding buffer
    writeTimer.start();
    itsArgs.BBuffer->writeElements((Beamlet *) &recvframe[itsArgs.frameHeaderSize], actualstamp, itsArgs.nTimesPerFrame);
    writeTimer.stop();
  }

  std::clog << "InputThread::mainLoop() exiting loop" << std::endl;
  if (pthread_join(logThreadId, 0) != 0) {
    std::cerr << "could not join log thread" << std::endl;
    exit(1);
  }
}

void InputThread::stopThreads()
{
  std::clog << "InputThread::stopThreads()" << std::endl;
  theirShouldStop = true;
}

} // namespace CS1
} // namespace LOFAR
