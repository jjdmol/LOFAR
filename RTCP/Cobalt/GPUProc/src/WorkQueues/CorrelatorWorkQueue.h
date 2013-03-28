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

#include <complex>

#include <Stream/Stream.h>
#include <CoInterface/Parset.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/SparseSet.h>

#include <GPUProc/global_defines.h>
#include <GPUProc/OpenCL_Support.h>
#include <GPUProc/FilterBank.h>
#include <GPUProc/SubbandMetaData.h>
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
    struct WorkQueueInputData
    {
      // Inputs: received from data and meta data
      MultiArraySharedBuffer<float, 3> delaysAtBegin;
      MultiArraySharedBuffer<float, 3> delaysAfterEnd;
      MultiArraySharedBuffer<float, 2> phaseOffsets;
      MultiArraySharedBuffer<char, 4> inputSamples;
      SparseSet<unsigned> flags;

      WorkQueueInputData(size_t n_beams, size_t n_stations, size_t n_polarizations,
                         size_t n_samples, size_t bytes_per_complex_sample,
                         cl::CommandQueue &queue, cl::Buffer &queue_buffer,
                         cl_mem_flags hostBufferFlags = CL_MEM_WRITE_ONLY, // input therefore assume host write, device read
                         cl_mem_flags deviceBufferFlags = CL_MEM_READ_ONLY)
        :
        delaysAtBegin(boost::extents[n_beams][n_stations][n_polarizations], queue, hostBufferFlags, deviceBufferFlags),
        delaysAfterEnd(boost::extents[n_beams][n_stations][n_polarizations], queue, hostBufferFlags, deviceBufferFlags),
        phaseOffsets(boost::extents[n_stations][n_polarizations], queue, hostBufferFlags, deviceBufferFlags),
        inputSamples(boost::extents[n_stations][n_samples][n_polarizations][bytes_per_complex_sample], queue, hostBufferFlags, queue_buffer) // TODO: The size of the buffer is NOT validated
      {
      }

      // Read for a station the data from the inputstream.
      // Perform the flagging of the data based on the just red meta data.
      void read(Stream *inputStream, size_t station, unsigned subband, unsigned beamIdx);

    private:
      void flagInputSamples(unsigned station, const SubbandMetaData& metaData);
    };

    class CorrelatorWorkQueue : public WorkQueue
    {
    public:
      CorrelatorWorkQueue(const Parset    &parset,cl::Context &context, cl::Device                &device, unsigned queueNumber,
                          CorrelatorPipelinePrograms &programs,
                          FilterBank &filterBank);

      void doWork();
      void doSubband(unsigned subband, CorrelatedData &output);
      //private:

      cl::Buffer devFIRweights;
      // Raw input buffer, to be mapped to a boost array
      cl::Buffer devCorrectedData;
      cl::Buffer devFilteredData;

      // static calculated/retrieved at the beginning
      MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights;

      // All input data collected in a single struct
      WorkQueueInputData inputData;

      // Output: received from the gpu and transfered from the metadata (flags)
      MultiArraySharedBuffer<std::complex<float>, 4> visibilities;

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
      //std::vector< WorkQueueInputItem> workQueueInputItems;
    private:
      void computeFlags(CorrelatedData &output);
    };

  }
}

#endif

