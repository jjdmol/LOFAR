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

#include <global_defines.h>
#include <OpenCL_Support.h>
#include <FilterBank.h>
#include <SubbandMetaData.h>
#include <Pipelines/CorrelatorPipelinePrograms.h>
#include <Kernels/FIR_FilterKernel.h>
#include <Kernels/Filter_FFT_Kernel.h>
#include <Kernels/DelayAndBandPassKernel.h>
#include <Kernels/CorrelatorKernel.h>

#include "WorkQueue.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class WorkQueueInputData
    {
      // Inputs: received from data and meta data
      MultiArraySharedBuffer<float, 3> delaysAtBegin;
      MultiArraySharedBuffer<float, 3> delaysAfterEnd;
      MultiArraySharedBuffer<float, 2> phaseOffsets;

      // inputdata with flagged data set to zero
      MultiArraySharedBuffer<char, 4> inputSamples;

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
      // Set data to zero based on the flags in the meta data
      void flagInputSamples(unsigned station, const SubbandMetaData& metaData);
    };


    // Propagate the flags
    unsigned get2LogOfNrChannels(unsigned nrChannels);
    void propagateFlagsToOutput(Parset const & parset,
        MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
        CorrelatedData &output);
    void convertFlagsToChannelFlags(Parset const &parset,
        MultiDimArray<LOFAR::SparseSet<unsigned>, 1>const &inputFlags,
        MultiDimArray<SparseSet<unsigned>, 2> &flagsPerChanel);

    void calculateAndSetNumberOfFlaggedSamples(Parset const &parset,
        MultiDimArray<SparseSet<unsigned>, 2>const & flagsPerChanel,
        CorrelatedData &output);

    void applyWeightingToAllPolarizations(unsigned baseline, 
        unsigned channel, float weight, CorrelatedData &output);

    void applyFractionOfFlaggedSamplesOnVisibilities(Parset const &parset,
        CorrelatedData &output);


    class CorrelatorWorkQueue : public WorkQueue
    {
    public:
      CorrelatorWorkQueue(const Parset &parset,cl::Context &context,
                          cl::Device &device, unsigned queueNumber,
                          CorrelatorPipelinePrograms &programs,
                          FilterBank &filterBank);

      void doWork();
      void doSubband(unsigned subband, CorrelatedData &output);
      //private:


    private:
      // Raw buffers, these are mapped with boost multiarrays 
      // in the InputData class
      cl::Buffer devCorrectedData;
      cl::Buffer devFilteredData;

    public:
      // All input data, flags and metadata collected in a single struct
      WorkQueueInputData inputData;

      // Output: received from the gpu and transfered from the metadata (flags)
      MultiArraySharedBuffer<std::complex<float>, 4> visibilities;

    private:
      // Compiled kernels
      cl::Buffer devFIRweights;
      FIR_FilterKernel firFilterKernel;
      Filter_FFT_Kernel fftKernel;
       // static calculated/retrieved at the beginning 
      MultiArraySharedBuffer<float, 1> bandPassCorrectionWeights;
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

