//# Pipeline.h
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
//# $Id$

#ifndef LOFAR_GPUPROC_PIPELINE_H
#define LOFAR_GPUPROC_PIPELINE_H

#include <string>
#include <vector>

#include <Common/LofarTypes.h>
#include <Common/Thread/Queue.h>
#include <Stream/Stream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/StreamableData.h>
#include <CoInterface/SmartPtr.h>

#include "Input/InputSection.h"
#include "Input/BeamletBufferToComputeNode.h"
#include "global_defines.h"
#include "OpenMP_Support.h"
#include "opencl-incl.h"
#include "createProgram.h"
#include "SlidingPointer.h"
#include "WorkQueues/WorkQueue.h"
#include "PerformanceCounter.h"

namespace LOFAR
{
  namespace Cobalt
  {

    template <typename SAMPLE_TYPE>
    class StationInput
    {
    public:
      SmartPtr<InputSection<SAMPLE_TYPE> >               inputSection;
      SmartPtr<BeamletBufferToComputeNode<SAMPLE_TYPE> > beamletBufferToComputeNode;

      void init(const Parset &ps, unsigned psetNumber);
    };

    template<typename SAMPLE_TYPE>
    void StationInput<SAMPLE_TYPE>::init(const Parset &ps, unsigned psetNumber)
    {
      string stationName = ps.getStationNamesAndRSPboardNumbers(psetNumber)[0].station;       // TODO: support more than one station
      std::vector<Parset::StationRSPpair> inputs = ps.getStationNamesAndRSPboardNumbers(psetNumber);

      inputSection = new InputSection<SAMPLE_TYPE>(ps, inputs);
      beamletBufferToComputeNode = new BeamletBufferToComputeNode<SAMPLE_TYPE>(ps, stationName, inputSection->itsBeamletBuffers, 0);
    }

    class Pipeline
    {
    public:
      Pipeline(const Parset &);

      cl::Program             createProgram(const char *sources);

      const Parset            &ps;
      cl::Context context;
      std::vector<cl::Device> devices;

      std::vector<StationInput<i16complex> > stationInputs16;       // indexed by station
      std::vector<StationInput<i8complex> >  stationInputs8;       // indexed by station
      std::vector<StationInput<i4complex> >  stationInputs4;       // indexed by station

      std::vector<SmartPtr<Stream> >  bufferToGPUstreams;       // indexed by station

      SlidingPointer<uint64_t> inputSynchronization;

#if defined USE_B7015
      OMP_Lock hostToDeviceLock[4], deviceToHostLock[4];
#endif

      void doWork();

      void                    sendNextBlock(unsigned station);
      
    protected:
      // combines all functionality needed for getting the total from a set of counters
      struct Performance {
        std::map<std::string, PerformanceCounter::figures> total_counters;
        std::map<std::string, SmartPtr<NSTimer> > total_timers;
        // lock on the shared data
        Mutex totalsMutex;
        // add the counter in this queue
        void addQueue(WorkQueue &queue);
        // Print a logline with results
        void log(size_t nrWorkQueues);
      } performance;
    };
  }
}

#endif

