//# InputThread.h: The thread that reads from a TH and places data into the buffer of the input section
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id: $

#ifndef LOFAR_GPUPROC_INPUT_THREAD_H
#define LOFAR_GPUPROC_INPUT_THREAD_H

// \file
// The thread that reads from a Stream and places data into the buffer of the input section

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <string>

#include <Common/Thread/Thread.h>
#include <Stream/Stream.h>
#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/SmartPtr.h>

#include "BeamletBuffer.h"
#include "LogThread.h"

namespace LOFAR
{
  namespace Cobalt
  {

    template<typename SAMPLE_TYPE>
    class InputThread
    {
    public:
      struct ThreadArgs {
        BeamletBuffer<SAMPLE_TYPE> *BBuffer;
        Stream              *stream;

        unsigned threadID;
        unsigned nrTimesPerPacket;
        unsigned nrSlotsPerPacket;
        LogThread::Counters *packetCounters;
        bool isRealTime;
        TimeStamp startTime;

        std::string logPrefix;
      };

      InputThread(ThreadArgs args);
      ~InputThread();

      void                  start();

      static const unsigned packetBuffersSize = 128;

    private:
      void                  mainLoop();

      ThreadArgs itsArgs;
      SmartPtr<Thread>      itsThread;
    };

  } // namespace Cobalt
} // namespace LOFAR

#endif

