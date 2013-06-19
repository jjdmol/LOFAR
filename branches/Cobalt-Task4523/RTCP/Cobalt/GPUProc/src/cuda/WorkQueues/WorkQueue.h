//# WorkQueue.h
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

#ifndef LOFAR_GPUPROC_CUDA_WORKQUEUE_H
#define LOFAR_GPUPROC_CUDA_WORKQUEUE_H

#include <string>
#include <map>

#include <Common/Timer.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/SubbandMetaData.h>
#include <CoInterface/StreamableData.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>

#include "Pool.h"

namespace LOFAR
{
  namespace Cobalt
  {
    // 
    //   Collect all inputData for the correlatorWorkQueue item:
    //    \arg inputsamples
    //    \arg delays
    //    \arg phaseOffSets
    //    \arg flags
    // It also contains a read function parsing all this data from an input stream.   
    class WorkQueueInputData
    {
    public:

      // The set of GPU buffers to link our host buffers to.
      // Device buffers may be reused between different pairs of kernels,
      // since device memory size is a concern. Use inputSamplesMinSize
      // to specify a minimum derived from other uses apart from input.
      struct DeviceBuffers
      {
        gpu::DeviceMemory delaysAtBegin;
        gpu::DeviceMemory delaysAfterEnd;
        gpu::DeviceMemory phaseOffsets;
        gpu::DeviceMemory inputSamples;

        DeviceBuffers(size_t inputSamplesSize, size_t delaysSize, 
                      size_t phaseOffsetsSize, gpu::Context &context) :
          delaysAtBegin(context, delaysSize),
          delaysAfterEnd(context, delaysSize),
          phaseOffsets(context, phaseOffsetsSize),
          inputSamples(context, inputSamplesSize)
        {
        }
      };

      // Which block this InputData represents
      struct BlockID blockID;

      //!< Whole sample delays at the start of the workitem      
      MultiDimArrayHostBuffer<float, 3> delaysAtBegin;

      //!< Whole sample delays at the end of the workitem      
      MultiDimArrayHostBuffer<float, 3> delaysAfterEnd;

      //!< Remainder of delays
      MultiDimArrayHostBuffer<float, 2> phaseOffsets;

      // inputdata with flagged data set to zero
      MultiDimArrayHostBuffer<char, 4> inputSamples;

      // The input flags
      MultiDimArray<SparseSet<unsigned>, 1> inputFlags;

      // Create the inputData object we need shared host/device memory on the supplied devicequeue
      WorkQueueInputData(size_t n_beams, size_t n_stations, size_t n_polarizations,
                         size_t n_samples, size_t bytes_per_complex_sample,
                         gpu::Context &context, unsigned int hostBufferFlags = 0)
        :
        delaysAtBegin(boost::extents[n_beams][n_stations][n_polarizations],
                       context, hostBufferFlags),
        delaysAfterEnd(boost::extents[n_beams][n_stations][n_polarizations],
                       context, hostBufferFlags),
        phaseOffsets(boost::extents[n_stations][n_polarizations],
                       context, hostBufferFlags),
        inputSamples(boost::extents[n_stations][n_samples][n_polarizations][bytes_per_complex_sample],
                       context, hostBufferFlags), // TODO: The size of the buffer is NOT validated
        inputFlags(boost::extents[n_stations])
      {
      }

      // process the given meta data 
      void applyMetaData(unsigned station, unsigned SAP, const SubbandMetaData &metaData);

      // set all flagged inputSamples to zero.
      void flagInputSamples(unsigned station, const SubbandMetaData& metaData);
    };

    class WorkQueue {
    public:
      WorkQueue(const Parset &ps, gpu::Context &context);
      virtual ~WorkQueue();

      // TODO: clean up access by Pipeline class and move under protected
      std::map<std::string, SmartPtr<PerformanceCounter> > counters;
      std::map<std::string, SmartPtr<NSTimer> > timers;

      class Flagger
      {
      public:
        // 1.1 Convert the flags per station to channel flags, change time scale if nchannel > 1
        static void convertFlagsToChannelFlags(Parset const &parset,
          MultiDimArray<SparseSet<unsigned>, 1> const &inputFlags,
          MultiDimArray<SparseSet<unsigned>, 2> &flagsPerChannel);

        // 1.3 Get the LOG2 of the input. Used to speed up devisions by 2
        static unsigned log2(unsigned n);
      };

      // A pool of input data, to allow items to be filled and
      // computed on in parallel.
      Pool<WorkQueueInputData> inputPool;

      // A pool of output data, to allow items to be filled
      // and written in parallel.
      Pool<StreamableData> outputPool;

      // Correlate the data found in the input data buffer
      virtual void processSubband(WorkQueueInputData &input, StreamableData &output);

      // Do post processing on the CPU
      virtual void postprocessSubband(StreamableData &output);

    protected:
      const Parset &ps;

      gpu::Stream queue;

      void addCounter(const std::string &name);
      void addTimer(const std::string &name);
    };
  }
}

#endif

