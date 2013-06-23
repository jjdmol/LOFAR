//# CorrelatorWorkQueue.h
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

#ifndef LOFAR_GPUPROC_CUDA_CORRELATOR_WORKQUEUE_H
#define LOFAR_GPUPROC_CUDA_CORRELATOR_WORKQUEUE_H

// @file
#include <complex>

#include <Common/Thread/Queue.h>
#include <Stream/Stream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/SmartPtr.h>
#include <CoInterface/SparseSet.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/FilterBank.h>
#include <GPUProc/Pipelines/CorrelatorPipelinePrograms.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/Filter_FFT_Kernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>

#include "WorkQueue.h"

namespace LOFAR
{
  namespace Cobalt
  {

    // A CorrelatedData object tied to a HostBuffer. Represents an output
    // data item that can be efficiently filled from the GPU.
    class CorrelatedDataHostBuffer: public MultiDimArrayHostBuffer<fcomplex, 4>,
                                    public CorrelatedData
    {
    public:
      CorrelatedDataHostBuffer(unsigned nrStations, unsigned nrChannels,
                               unsigned maxNrValidSamples, gpu::Context &context)
      :
        MultiDimArrayHostBuffer<fcomplex, 4>(boost::extents[nrStations * (nrStations + 1) / 2]
                                                           [nrChannels][NR_POLARIZATIONS]
                                                           [NR_POLARIZATIONS], context, 0),
        CorrelatedData(nrStations, nrChannels, maxNrValidSamples, this->origin(),
                       this->num_elements(), heapAllocator, 1)
      {
      }
    };

    class CorrelatorWorkQueue : public WorkQueue
    {
    public:
      CorrelatorWorkQueue(const Parset &parset, gpu::Context &context,
                          CorrelatorPipelinePrograms &programs,
                          FilterBank &filterBank);

      // Correlate the data found in the input data buffer
      virtual void processSubband(WorkQueueInputData &input, StreamableData &output);

      // Do post processing on the CPU
      virtual void postprocessSubband(StreamableData &output);

      // Collection of functions to tranfer the input flags to the output.
      // \c propagateFlags can be called parallel to the kernels.
      // After the data is copied from the the shared buffer 
      // \c applyWeights can be used to weight the visibilities 
      class Flagger: public WorkQueue::Flagger
      {
      public:
        // 1. Convert input flags to channel flags, calculate the amount flagged samples and save this in output
        static void propagateFlags(Parset const & parset,
          MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
          CorrelatedData &output);

        // 2. Calculate the weight based on the number of flags and apply this weighting to all output values
        template<typename T> static void applyWeights(Parset const &parset, CorrelatedData &output);

        // 1.2 Calculate the number of flagged samples and set this on the output dataproduct
        // This function is aware of the used filter width a corrects for this.
        template<typename T> static void calcWeights(Parset const &parset,
          MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
          CorrelatedData &output);

        // 2.1 Apply the supplied weight to the complex values in the channel and baseline
        static void applyWeight(unsigned baseline, unsigned channel, float weight, CorrelatedData &output);
      };
      
    private:
      // The previously processed SAP/block, or -1 if nothing has been
      // processed yet. Used in order to determine if new delays have
      // to be uploaded.
      ssize_t prevBlock;
      signed int prevSAP;

      // Raw buffers, these are mapped with boost multiarrays 
      // in the InputData class
      WorkQueueInputData::DeviceBuffers devInput;

      gpu::DeviceMemory devFilteredData;

      // Constant input buffers for the kernels
      gpu::DeviceMemory devFIRweights;

      // Compiled kernels
      FIR_FilterKernel firFilterKernel;
      Filter_FFT_Kernel fftKernel;
      DelayAndBandPassKernel delayAndBandPassKernel;
#if defined USE_NEW_CORRELATOR
      CorrelateTriangleKernel correlateTriangleKernel;
      CorrelateRectangleKernel correlateRectangleKernel;
#else
      CorrelatorKernel correlatorKernel;
#endif
    };

  }
}

#endif

