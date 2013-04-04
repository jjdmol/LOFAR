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

#ifndef LOFAR_GPUPROC_CORRELATOR_WORKQUEUE_H
#define LOFAR_GPUPROC_CORRELATOR_WORKQUEUE_H

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
#include <GPUProc/OpenCL_Support.h>
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
    // The pool operates using a 'free' and a 'filled' queue to cycle through buffers. Producers
    // move elements free->filled, and consumers move elements filled->free.
    template <typename T>
    struct Pool
    {
      typedef T element_type;

      Queue< SmartPtr<element_type> > free;
      Queue< SmartPtr<element_type> > filled;
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

      // The set of GPU buffers to link our HostBuffers to.
      struct DeviceBuffers
      {
        DeviceBuffer delaysAtBegin;
        DeviceBuffer delaysAfterEnd;
        DeviceBuffer phaseOffsets;
        DeviceBuffer inputSamples;

        DeviceBuffers(size_t n_beams, size_t n_stations, size_t n_polarizations,
                         size_t n_samples, size_t bytes_per_complex_sample,
                         cl::CommandQueue &queue,
                         size_t inputSamplesMinSize = 0,
                         cl_mem_flags deviceBufferFlags = CL_MEM_READ_ONLY)
        :
          delaysAtBegin(queue, deviceBufferFlags, n_beams * n_stations * n_polarizations * sizeof(float)),
          delaysAfterEnd(queue, deviceBufferFlags, n_beams * n_stations * n_polarizations * sizeof(float)),
          phaseOffsets(queue, deviceBufferFlags, n_stations * n_polarizations * sizeof(float)),
          inputSamples(queue, CL_MEM_READ_WRITE, std::max(inputSamplesMinSize, n_stations * n_samples * n_polarizations * bytes_per_complex_sample))
        {
        }
      };

      MultiArrayHostBuffer<float, 3> delaysAtBegin; //!< Whole sample delays at the start of the workitem      
      MultiArrayHostBuffer<float, 3> delaysAfterEnd;//!< Whole sample delays at the end of the workitem      
      MultiArrayHostBuffer<float, 2> phaseOffsets;  //!< Remainder of delays

      // inputdata with flagged data set to zero
      MultiArrayHostBuffer<char, 4> inputSamples;

      // The input flags
      MultiDimArray<SparseSet<unsigned>,1> inputFlags;

      // Create the inputData object we need shared host/device memory on the supplied devicequeue
      WorkQueueInputData(size_t n_beams, size_t n_stations, size_t n_polarizations,
                         size_t n_samples, size_t bytes_per_complex_sample,
                         DeviceBuffers &deviceBuffers,
                         cl_mem_flags hostBufferFlags = CL_MEM_WRITE_ONLY)
        :
        delaysAtBegin(boost::extents[n_beams][n_stations][n_polarizations], hostBufferFlags, deviceBuffers.delaysAtBegin),
        delaysAfterEnd(boost::extents[n_beams][n_stations][n_polarizations], hostBufferFlags, deviceBuffers.delaysAfterEnd),
        phaseOffsets(boost::extents[n_stations][n_polarizations], hostBufferFlags, deviceBuffers.phaseOffsets),
        inputSamples(boost::extents[n_stations][n_samples][n_polarizations][bytes_per_complex_sample], hostBufferFlags, deviceBuffers.inputSamples), // TODO: The size of the buffer is NOT validated
        inputFlags(boost::extents[n_stations])
      {
      }

      // Read for \c station the data and metadata from the \c inputstream, set flagged samples zero.
      void read(Stream *inputStream, size_t station, unsigned subband, unsigned beamIdx);

    private:
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
      class flagFunctions
      {
      public:
        // 1. Convert input flags to channel flags, calculate the amount flagged samples and save this in output
        static void propagateFlagsToOutput(Parset const & parset,
          MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
          CorrelatedData &output) ;

        // 2. Calculate the weight based on the number of flags and apply this weighting to all output values
        static void applyFractionOfFlaggedSamplesOnVisibilities(Parset const &parset,
          CorrelatedData &output);

        // 1.1Convert the flags per station to channel flags, change time scale if nchannel > 1
        static void convertFlagsToChannelFlags(Parset const &parset,
          MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
          MultiDimArray<SparseSet<unsigned>, 2> &flagsPerChanel);

        // 1.2calculate the number of flagged samples and set this on the output dataproduct
        // This function is aware of the used filter width a corrects for this.
        static void calculateAndSetNumberOfFlaggedSamples(Parset const &parset,
          MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChanel,
          CorrelatedData &output);

        // 1.3 Get the LOG2 of the input. Used to speed up devisions by 2
        static unsigned get2LogOfNrChannels(unsigned nrChannels);

        // 2.1 Apply the supplied weight to the complex values in the channel and baseline
        static void applyWeightingToAllPolarizations(unsigned baseline, 
          unsigned channel, float weight, CorrelatedData &output);
      };

    public:
      CorrelatorWorkQueue(const Parset &parset,cl::Context &context,
                          cl::Device &device, unsigned queueNumber,
                          CorrelatorPipelinePrograms &programs,
                          FilterBank &filterBank);

      // Correlate the data found in the input data buffer
      void doSubband(unsigned subband, CorrelatedData &output);
      
    private:
      // Raw buffers, these are mapped with boost multiarrays 
      // in the InputData class
      WorkQueueInputData::DeviceBuffers devInput;

      DeviceBuffer devFilteredData;

    public:
      // All input data collected in a single struct
      Pool<WorkQueueInputData> inputPool;

      // the visibilities 
      MultiArrayHostBuffer<std::complex<float>, 4> visibilities;

    private:
      // Compiled kernels
      DeviceBuffer devFIRweights;
      FIR_FilterKernel firFilterKernel;
      Filter_FFT_Kernel fftKernel;
      MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights;
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
