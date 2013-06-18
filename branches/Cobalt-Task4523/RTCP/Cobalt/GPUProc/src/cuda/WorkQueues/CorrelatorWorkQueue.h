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
#include <CoInterface/SubbandMetaData.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/BlockID.h>
#include <GPUProc/FilterBank.h>
#include <GPUProc/Pipelines/CorrelatorPipelinePrograms.h>
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <GPUProc/Kernels/Filter_FFT_Kernel.h>
#include <GPUProc/Kernels/DelayAndBandPassKernel.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>

#include "WorkQueue.h"
#include "Pool.h"

namespace LOFAR
{
  namespace Cobalt
  {
    /*
     * The CorrelatorWorkQueue does the following transformation:
     *   WorkQueueInputData -> CorrelatedDataHostBuffer
     *
     * The WorkQueueInputData represents one block of one subband
     * of input data, and the CorrelatedDataHostBuffer the complex
     * visibilities of such a block.
     *
     * For both input and output, a fixed set of objects is created,
     * tied to the GPU specific for the WorkQueue, for increased
     * performance. The objects are recycled by using Pool objects.
     *
     * The data flows as follows:
     *
     *   // Fetch the next input object to fill
     *   SmartPtr<WorkQueueInputData> input = queue.inputPool.free.remove();
     *
     *   // Provide input
     *   receiveInput(input);
     *
     *   // Annotate input
     *   input->blockID.block = block;
     *   input->blockID.globalSubbandIdx = subband;
     *   input->blockID.localSubbandIdx  = subbandIdx;
     *
     *   // Fetch the next output object to fill
     *   SmartPtr<CorrelatedDataHostBuffer> output = queue.outputPool.free.remove();
     *
     *   // Process block
     *   queue.doSubband(input, output);
     *
     *   // Give back input and output objects to queue
     *   queue.inputPool.free.append(input);
     *   queue.outputPool.free.append(output);
     *
     *   The queue.inputPool.filled and queue.outputPool.filled can be used to
     *   temporarily store filled input and output objects. Such is needed to
     *   obtain parallellism (i.e. read/process/write in separate threads).
     */
    class CorrelatorWorkQueue;

    // A CorrelatedData object tied to a HostBuffer and WorkQueue. Such links
    // After the visibilities have been written to storage, we need remember
    // the queue to recycle the buffer.
    class CorrelatedDataHostBuffer: public MultiDimArrayHostBuffer<fcomplex, 4>,
                                    public CorrelatedData
    {
    public:
      CorrelatedDataHostBuffer(unsigned nrStations, unsigned nrChannels,
                               unsigned maxNrValidSamples, gpu::Context &context,
                               CorrelatorWorkQueue &workQueue)
      :
        MultiDimArrayHostBuffer<fcomplex, 4>(boost::extents[nrStations * (nrStations + 1) / 2]
                                                           [nrChannels][NR_POLARIZATIONS]
                                                           [NR_POLARIZATIONS], context, 0),
        CorrelatedData(nrStations, nrChannels, maxNrValidSamples, this->origin(),
                       this->num_elements(), heapAllocator, 1),
        workQueue(workQueue)
      {
      }

      // Annotation required, as we'll lose track of the exact order
      struct BlockID blockID;

      CorrelatorWorkQueue &workQueue;

    private:
      CorrelatedDataHostBuffer();
      CorrelatedDataHostBuffer(const CorrelatedDataHostBuffer &);
    };

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
        // DeviceBuffers(size_t n_beams, size_t n_stations, size_t n_polarizations,
        //               size_t n_samples, size_t bytes_per_complex_sample,
        //               gpu::Context &context, size_t inputSamplesMinSize = 0)
        // :
        //   delaysAtBegin (context, n_beams * n_stations * n_polarizations * sizeof(float)),
        //   delaysAfterEnd(context, n_beams * n_stations * n_polarizations * sizeof(float)),
        //   phaseOffsets  (context,           n_stations * n_polarizations * sizeof(float)),
        //   inputSamples  (context, std::max(inputSamplesMinSize,
        //                         n_samples * n_stations * n_polarizations * bytes_per_complex_sample))
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

    class CorrelatorWorkQueue : public WorkQueue
    {
    public:
      // Collection of functions to tranfer the input flags to the output.
      // \c propagateFlagsToOutput can be called parallel to the kernels.
      // After the data is copied from the the shared buffer 
      // \c applyFractionOfFlaggedSamplesOnVisibilities can be used to weight
      // the visibilities 
      class Flagger: public WorkQueue::Flagger
      {
      public:
        // 1. Convert input flags to channel flags, calculate the amount flagged samples and save this in output
        static void propagateFlagsToOutput(Parset const & parset,
          MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
          CorrelatedData &output);

        // 2. Calculate the weight based on the number of flags and apply this weighting to all output values
        template<typename T> static void applyFractionOfFlaggedSamplesOnVisibilities(Parset const &parset,
          CorrelatedData &output);

        // 1.2 Calculate the number of flagged samples and set this on the output dataproduct
        // This function is aware of the used filter width a corrects for this.
        template<typename T> static void calculateAndSetNumberOfFlaggedSamples(Parset const &parset,
          MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChannel,
          CorrelatedData &output);

        // 2.1 Apply the supplied weight to the complex values in the channel and baseline
        static void applyWeightingToAllPolarizations(unsigned baseline, 
          unsigned channel, float weight, CorrelatedData &output);
      };

    public:
      CorrelatorWorkQueue(const Parset &parset, gpu::Context &context,
                          CorrelatorPipelinePrograms &programs,
                          FilterBank &filterBank);

      // Correlate the data found in the input data buffer
      void processSubband(WorkQueueInputData &input, CorrelatedDataHostBuffer &output);

      // Do post processing on the CPU
      void postprocessSubband(CorrelatedDataHostBuffer &output);
      
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

    public:
      // A pool of input data, to allow items to be filled and
      // computed on in parallel.
      Pool<WorkQueueInputData> inputPool;

      // A pool of output data, to allow items to be filled
      // and written in parallel.
      Pool<CorrelatedDataHostBuffer> outputPool;

    private:
      // Constant input buffers for the kernels
      gpu::DeviceMemory devFIRweights;
      gpu::DeviceMemory devBandPassCorrectionWeights;

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

      friend class WorkQueueInputData;
    };

  }
}

#endif

