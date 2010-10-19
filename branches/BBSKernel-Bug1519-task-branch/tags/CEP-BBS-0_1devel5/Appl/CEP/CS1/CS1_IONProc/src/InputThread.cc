//#  InputThread.cc: the thread that reads from a Stream and places data into
//#  the buffer of the input section
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
#include <CS1_Interface/AlignedStdAllocator.h>
#include <Stream/NullStream.h>
#include <Stream/SystemCallException.h>
#include <BeamletBuffer.h>
#include <InputThread.h>
#include <RSP.h>

#include <errno.h>
#include <signal.h>

#include <cstddef>

#include <boost/multi_array.hpp>


namespace LOFAR {
namespace CS1 {


InputThread::InputThread(ThreadArgs args /* call by value! */)
:
  itsArgs(args)
{
  std::clog << "InputThread::InputThread(...)" << std::endl;

  struct sigaction sa;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags	= 0;
  sa.sa_handler = sigHandler;

  if (sigaction(SIGUSR1, &sa, 0) != 0) {
    perror("sigaction");
    exit(1);
  }

  stop = stopped = false;

  if (pthread_create(&thread, 0, mainLoopStub, this) != 0) {
    std::cerr << "could not create input thread" << std::endl;
    exit(1);
  }
}


InputThread::~InputThread()
{
  std::clog << "InputThread::~InputThread()" << std::endl;

  stop = true;

  while (!stopped) {
    if (pthread_kill(thread, SIGUSR1) != 0) { // interrupt read() system call
      perror("pthread_kill");
      exit(1);
    }

    usleep(25000);
  }

  if (pthread_join(thread, 0) != 0) {
    std::cerr << "could not join input thread" << std::endl;
    exit(1);
  }
}


void InputThread::sigHandler(int)
{
}


void InputThread::setAffinity()
{
#if 1 && __linux__
  cpu_set_t cpu_set;

  CPU_ZERO(&cpu_set);

  for (unsigned cpu = 1; cpu < 4; cpu ++)
    CPU_SET(cpu, &cpu_set);

  if (sched_setaffinity(0, sizeof cpu_set, &cpu_set) != 0) {
    std::clog << "WARNING: sched_setaffinity failed" << std::endl;
    perror("sched_setaffinity");
  }
#endif
}


void *InputThread::mainLoopStub(void *inputThread)
{
  try {
    static_cast<InputThread *>(inputThread)->mainLoop();
  } catch (Exception &ex) {
    std::cerr << "caught Exception: " << ex.what() << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "caught non-std:exception" << std::endl;
  }

  static_cast<InputThread *>(inputThread)->stopped = true;
  return 0;
}


void InputThread::mainLoop()
{
  setAffinity();

  const unsigned maxNrPackets = 128;
  TimeStamp	 actualstamp  = itsArgs.startTime - itsArgs.nrTimesPerPacket;
  unsigned	 packetSize   = sizeof(struct RSP::header) + itsArgs.nrSubbandsPerPacket * itsArgs.nrTimesPerPacket * sizeof(Beamlet);

  std::vector<TimeStamp> timeStamps(maxNrPackets);
  boost::multi_array<char, 2, AlignedStdAllocator<char, 32> > packets(boost::extents[maxNrPackets][packetSize]);

  char		*currentPacketPtr	    = packets.origin();
  unsigned	currentPacket		    = 0;

  unsigned	previousSeqid		    = 0;
  bool		previousSeqidIsAccepted	    = false;
  
  bool		dataShouldContainValidStamp = dynamic_cast<NullStream *>(itsArgs.stream) == 0;
  WallClockTime wallClockTime;

  std::clog << "input thread " << itsArgs.threadID << " entering loop" << std::endl;

  while (!stop) {
    try {
      // interruptible read, to allow stopping this thread even if the station
      // does not send data

      itsArgs.stream->read(currentPacketPtr, packetSize);
    } catch (SystemCallException &ex) {
      if (ex.error == EINTR)
	break;
      else
	throw ex;
    }

    ++ itsArgs.packetCounters->nrPacketsReceived;

    if (dataShouldContainValidStamp) {
#if defined __PPC__
      unsigned seqid, blockid;

      asm volatile ("lwbrx %0,%1,%2" : "=r" (seqid)   : "b" (currentPacketPtr), "r" (offsetof(RSP, header.timestamp)));
      asm volatile ("lwbrx %0,%1,%2" : "=r" (blockid) : "b" (currentPacketPtr), "r" (offsetof(RSP, header.blockSequenceNumber)));
#else
      unsigned seqid   = reinterpret_cast<RSP *>(currentPacketPtr)->header.timestamp;
      unsigned blockid = reinterpret_cast<RSP *>(currentPacketPtr)->header.blockSequenceNumber;

#if defined WORDS_BIGENDIAN
      seqid   = byteSwap(seqid);
      blockid = byteSwap(blockid);
#endif
#endif

      //if the seconds counter is 0xFFFFFFFF, the data cannot be trusted.
      if (seqid == ~0U) {
	++ itsArgs.packetCounters->nrPacketsRejected;
	continue;
      }

      // Sanity check on seqid. Note, that seqid is in seconds,
      // so a value which is greater than the previous one with more 
      // than (say) 10 seconds probably means that the sequence number 
      // in the packet is wrong. This can happen, since communication is not
      // reliable.
      if (seqid >= previousSeqid + 10 && previousSeqidIsAccepted) {
	previousSeqidIsAccepted = false;
	++ itsArgs.packetCounters->nrPacketsRejected;
	continue;
      }

      // accept seqid
      previousSeqidIsAccepted = true;
      previousSeqid	      = seqid;
	
      actualstamp.setStamp(seqid, blockid);
    } else {
      actualstamp += itsArgs.nrTimesPerPacket; 

      if (itsArgs.isRealTime)
	wallClockTime.waitUntil(actualstamp);
    }

    // expected packet received so write data into corresponding buffer
    //itsArgs.BBuffer->writePacketData(reinterpret_cast<Beamlet *>(&packet.data), actualstamp);

    timeStamps[currentPacket] = actualstamp;
    currentPacketPtr += packetSize;

    if (++ currentPacket == maxNrPackets) {
      itsArgs.BBuffer->writeMultiplePackets(packets.origin(), timeStamps);
      currentPacket    = 0;
      currentPacketPtr = packets.origin();
    }
  }

  std::clog << "InputThread::mainLoop() exiting loop" << std::endl;
}

} // namespace CS1
} // namespace LOFAR
