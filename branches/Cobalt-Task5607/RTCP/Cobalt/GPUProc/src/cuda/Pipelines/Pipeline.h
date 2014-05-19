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

#ifndef LOFAR_GPUPROC_CUDA_PIPELINE_H
#define LOFAR_GPUPROC_CUDA_PIPELINE_H

#include <string>
#include <vector>
#include <map>
#include <time.h>

#include <Common/LofarTypes.h>
#include <Common/Thread/Queue.h>
#include <Common/Thread/Mutex.h>
#include <CoInterface/BestEffortQueue.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/SlidingPointer.h>
#include <CoInterface/Pool.h>
#include <CoInterface/OMPThread.h>

#include <InputProc/Transpose/MPIUtil.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/OpenMP_Lock.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/SubbandProcs/SubbandProc.h>

#include <GPUProc/MPI_utils.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class Pipeline
    {
    public:
      Pipeline(const Parset &ps, const std::vector<size_t> &subbandIndices, const std::vector<gpu::Device> &devices);

      virtual ~Pipeline();

      // allocate resources, such as GPU buffers.
      //
      // Pipeline deploys delayed construction, because the resources may still
      // be occupied by a previous observation at the time of construction.
      //
      // An alternative to delayed allocation could be to retry the GPU malloc
      // with a timeout, but that could potentially dead-lock two concurrent
      // observations.
      virtual void allocateResources();

      // for each subband get data from input stream, sync, start the kernels to process all data, write output in parallel
      virtual void processObservation();

      struct Output 
      {
        // output data queue
        SmartPtr< BestEffortQueue< SmartPtr<SubbandProcOutputData> > > bequeue;
      };

    protected:
      const Parset             &ps;
      const std::vector<gpu::Device> devices;

      const std::vector<size_t> subbandIndices; // [localSubbandIdx]

      // Whether we're the pipeline that processes the first subband.
      // If true, we log our progress at INFO. Otherwise, at DEBUG.
      const bool processingSubband0;

      std::vector< SmartPtr<SubbandProc> > workQueues;

      const size_t nrSubbandsPerSubbandProc;

    protected:
      // Threads that write to outputProc, and need to
      // be killed when they stall at observation end.
      OMPThreadSet outputThreads;

    private:

      Pool<struct MPIRecvData> mpiPool;

      // For each block, read all data and put it (untransposed) in the mpiPool
      void receiveInput( size_t nrBlocks );
      template<typename SampleT> void receiveInput( size_t nrBlocks );

      // For each block, transpose all subbands from all stations, and divide the
      // work over the workQueues
      void transposeInput();
      template<typename SampleT> void transposeInput();

      // preprocess subbands on the CPU
      void preprocessSubbands(SubbandProc &workQueue);

      // process subbands on the GPU
      void processSubbands(SubbandProc &workQueue);

      // Post-process subbands on the CPU
      void postprocessSubbands(SubbandProc &workQueue);

      // Send subbands to Storage
      virtual void writeOutput(unsigned globalSubbandIdx, struct Output &output) = 0;

      // Signal that all output has been emitted
      virtual void doneWritingOutput();

      std::vector<struct Output> writePool; // [localSubbandIdx]
    };
  }
}

#endif

