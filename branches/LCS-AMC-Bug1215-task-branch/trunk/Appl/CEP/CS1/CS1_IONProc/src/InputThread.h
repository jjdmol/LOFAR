//#  InputThread.h: The thread that reads from a TH and places data into the buffer of the input section
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

#ifndef LOFAR_CS1_INPUTSECTION_INPUTTHREAD_H
#define LOFAR_CS1_INPUTSECTION_INPUTTHREAD_H

// \file
// The thread that reads from a Stream and places data into the buffer of the input section

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_vector.h>
#include <CS1_Interface/RSPTimeStamp.h>
#include <BeamletBuffer.h>
#include <LogThread.h>

#include <pthread.h>

namespace LOFAR {
namespace CS1 {

  struct ThreadArgs {
    BeamletBuffer	*BBuffer;
    Stream		*stream; 

    unsigned		threadID;
    unsigned		frameSize;
    unsigned		ipHeaderSize;
    unsigned		frameHeaderSize;
    unsigned		nTimesPerFrame;
    unsigned		nSubbandsPerFrame;
    LogThread::Counters	*packetCounters;
    bool		isRealTime;
    TimeStamp		startTime;
  };

class InputThread
{
  public:
    InputThread(ThreadArgs args);
    ~InputThread();

    static void *mainLoopStub(void *inputThread);
    void	  mainLoop();

  private:
    static void sigHandler(int);
  
    static volatile bool theirShouldStop;

    ThreadArgs itsArgs;
    pthread_t  thread;
  };

} // namespace CS1
} // namespace LOFAR

#endif
