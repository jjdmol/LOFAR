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

#include <Common/LofarTypes.h>
#include <MACIO/RTmetadata.h>
#include <CoInterface/Parset.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/Pool.h>
#include <CoInterface/OMPThread.h>
#include <CoInterface/TABTranspose.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/PerformanceCounter.h>
#include <GPUProc/SubbandProcs/SubbandProc.h>
#include <GPUProc/SubbandProcs/KernelFactories.h>

#include <GPUProc/MPIReceiver.h>

namespace LOFAR
{
  namespace Cobalt
  {
    using MACIO::RTmetadata;

    class Pipeline
    {
    public:
      Pipeline(const Parset &, const std::vector<size_t> &subbandIndices, 
        const std::vector<gpu::Device> &devices,
        Pool<struct MPIRecvData> &pool,
        RTmetadata &mdLogger, const std::string &mdKeyPrefix,
        int hostID = 0);

      ~Pipeline();

      // allocate resources, such as GPU buffers.
      //
      // Pipeline deploys delayed construction, because the resources may still
      // be occupied by a previous observation at the time of construction.
      //
      // An alternative to delayed allocation could be to retry the GPU malloc
      // with a timeout, but that could potentially dead-lock two concurrent
      // observations.
      void allocateResources();

      // for each subband get data from input stream, sync, start the kernels to process all data, write output in parallel
      void processObservation();

      struct Output 
      {
        // output data queue
        SmartPtr< Queue< SmartPtr<SubbandProcOutputData> > > queue;
      };

      std::vector< SmartPtr<SubbandProc> > subbandProcs;

    protected:
      const Parset             &ps;
      const std::vector<gpu::Device> devices;

      const std::vector<size_t> subbandIndices; // [localSubbandIdx]

      // Whether we're the pipeline that processes the first subband.
      // If true, we log our progress at INFO. Otherwise, at DEBUG.
      const bool processingSubband0;

      const size_t nrSubbandsPerSubbandProc;

      RTmetadata &itsMdLogger; // non-const to be able to use its log()
      const std::string itsMdKeyPrefix;

      // Threads that write to outputProc, and need to
      // be killed when they stall at observation end.
      OMPThreadSet outputThreads;

      Pool<struct MPIRecvData> &mpiPool;

      std::vector<struct Output> writePool; // [localSubbandIdx]

      KernelFactories factories;

      // For each block, transpose all subbands from all stations, and divide the
      // work over the subbandProcs
      void transposeInput();
      template<typename SampleT> void transposeInput();

      // preprocess subbands on the CPU
      void preprocessSubbands(SubbandProc &subbandProc);

      // process subbands on the GPU
      void processSubbands(SubbandProc &subbandProc);

      // Post-process subbands on the CPU
      void postprocessSubbands(SubbandProc &subbandProc);

      void writeBeamformedOutput(
        unsigned globalSubbandIdx,
        Queue< SmartPtr<SubbandProcOutputData> > &inputQueue,
        Queue< SmartPtr<SubbandProcOutputData> > &outputQueue,
        Queue< SmartPtr<SubbandProcOutputData> > &spillQueue );

      void writeCorrelatedOutput(
        unsigned globalSubbandIdx,
        Queue< SmartPtr<SubbandProcOutputData> > &inputQueue,
        Queue< SmartPtr<SubbandProcOutputData> > &outputQueue );

    public:
      // Send subbands to Storage
      void writeOutput(
        unsigned globalSubbandIdx,
        Queue< SmartPtr<SubbandProcOutputData> > &inputQueue,
        Queue< SmartPtr<SubbandProcOutputData> > &outputQueue );

      // Output send engine, takes care of the host connections and the multiplexing.
      TABTranspose::MultiSender multiSender;
    };
  }
}

#endif

