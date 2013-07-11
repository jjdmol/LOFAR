//# FIR_FilterKernel.cc
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

#include <lofar_config.h>

#include "FIR_FilterKernel.h"

#include <Common/lofar_complex.h>

#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    FIR_FilterKernel::FIR_FilterKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, "FIR_filter")),
      devFIRweights(buffers.filterWeights)
    {
      init(stream, buffers, params);
    }

    FIR_FilterKernel::FIR_FilterKernel(const Parset &ps, 
                                       gpu::Context &context,
                                       gpu::DeviceMemory &devFilteredData,
                                       gpu::DeviceMemory &devInputSamples,
                                       gpu::Stream &stream)
      :
      Kernel(ps, context, "FIR_Filter.cu", "FIR_filter"),
      devFIRweights(context, bufferSize(ps, FILTER_WEIGHTS))
    {
      // Parameters initialiseren o.b.v. Parset ...
      init(devFilteredData, devInputSamples, stream);
    }

    FIR_FilterKernel::FIR_FilterKernel(const Parset &ps, 
                                       gpu::Module &module,
                                       gpu::DeviceMemory &devFilteredData,
                                       gpu::DeviceMemory &devInputSamples,
                                       gpu::Stream &stream)
      :
      Kernel(ps, module, "FIR_filter"),
      devFIRweights(module.getContext(), bufferSize(ps, FILTER_WEIGHTS))
    {
      // Parameters initialiseren o.b.v. Parset ...
      init(devFilteredData, devInputSamples, stream);
    }

    void FIR_FilterKernel::init(const gpu::Stream &stream,
                                const Buffers &buffers,
                                const Parameters& params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, devFIRweights);

      size_t maxNrThreads = 
        getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);

      unsigned totalNrThreads = 
        params.nrChannelsPerSubband * params.nrPolarizations * 2;
      unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;

      globalWorkSize = gpu::Grid(totalNrThreads, params.nrStations);
      localWorkSize = gpu::Block(totalNrThreads / nrPasses, 1);

      size_t nrSamples = 
        params.nrStations * params.nrChannelsPerSubband * 
        params.nrPolarizations;

      nrOperations = 
        nrSamples * params.nrSamplesPerChannel * params.nrPPFTaps * 2 * 2;

      nrBytesRead = 
        nrSamples * (params.nrPPFTaps - 1 + params.nrSamplesPerChannel) * 
        params.nrBytesPerComplexSample;

      nrBytesWritten = 
        nrSamples * params.nrSamplesPerChannel * sizeof(std::complex<float>);

      // Note that these constant weights are now (unnecessarily) stored on the
      // device for every workqueue. A single copy per device could be used, but
      // first verify that the device platform still allows workqueue overlap.
      FilterBank filterBank(true, params.nrPPFTaps, 
                            params.nrChannelsPerSubband, KAISER);
      filterBank.negateWeights();

      gpu::HostMemory firWeights(stream.getContext(), devFIRweights.size());
      std::memcpy(firWeights.get<void>(), filterBank.getWeights().origin(),
                  firWeights.size());
      stream.writeBuffer(devFIRweights, firWeights, true);
    }


    void FIR_FilterKernel::init(gpu::DeviceMemory &devFilteredData,
                                gpu::DeviceMemory &devInputSamples,
                                gpu::Stream &stream)
    {
      setArg(0, devFilteredData);
      setArg(1, devInputSamples);
      setArg(2, devFIRweights);

      size_t maxNrThreads;
      maxNrThreads = getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);
      unsigned totalNrThreads = ps.nrChannelsPerSubband() * NR_POLARIZATIONS * 2;
      unsigned nrPasses = (totalNrThreads + maxNrThreads - 1) / maxNrThreads;
      globalWorkSize = gpu::Grid(totalNrThreads, ps.nrStations());
      localWorkSize = gpu::Block(totalNrThreads / nrPasses, 1);

      size_t nrSamples = (size_t) ps.nrStations() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS;
      nrOperations = nrSamples * ps.nrSamplesPerChannel() * NR_TAPS * 2 * 2;
      nrBytesRead = nrSamples * (NR_TAPS - 1 + ps.nrSamplesPerChannel()) * ps.nrBytesPerComplexSample();
      nrBytesWritten = nrSamples * ps.nrSamplesPerChannel() * sizeof(std::complex<float>);

      // Note that these constant weights are now (unnecessarily) stored on the
      // device for every workqueue. A single copy per device could be used, but
      // first verify that the device platform still allows workqueue overlap.
      FilterBank filterBank(true, NR_TAPS, ps.nrChannelsPerSubband(), KAISER);
      filterBank.negateWeights();

      gpu::HostMemory firWeights(stream.getContext(), devFIRweights.size());
      std::memcpy(firWeights.get<void>(), filterBank.getWeights().origin(), firWeights.size());
      stream.writeBuffer(devFIRweights, firWeights, true);
    }

    size_t FIR_FilterKernel::bufferSize(const Parset& ps, BufferType bufferType)
    {
      switch (bufferType) {
      case INPUT_DATA: 
        return
          (ps.nrHistorySamples() + ps.nrSamplesPerSubband()) * 
          ps.nrStations() * NR_POLARIZATIONS * ps.nrBytesPerComplexSample();
      case OUTPUT_DATA:
        return
          ps.nrSamplesPerSubband() * ps.nrStations() * 
          NR_POLARIZATIONS * sizeof(std::complex<float>);
      case FILTER_WEIGHTS:
        return 
          ps.nrChannelsPerSubband() * NR_TAPS * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }


    template<>
    KernelFactory<FIR_FilterKernel>::KernelFactory(const Parset& ps) :
      itsParset(ps)
    {
      // Compile FIR_FilterKernel source to PTX
      // itsPTX = ...
    }

    template<> size_t 
    KernelFactory<FIR_FilterKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case FIR_FilterKernel::INPUT_DATA: 
        return
          (itsParset.nrHistorySamples() + itsParset.nrSamplesPerSubband()) * 
          itsParset.nrStations() * NR_POLARIZATIONS * 
          itsParset.nrBytesPerComplexSample();
      case FIR_FilterKernel::OUTPUT_DATA:
        return
          itsParset.nrSamplesPerSubband() * itsParset.nrStations() * 
          NR_POLARIZATIONS * sizeof(std::complex<float>);
      case FIR_FilterKernel::FILTER_WEIGHTS:
        return 
          itsParset.nrChannelsPerSubband() * NR_TAPS * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

  }
}
