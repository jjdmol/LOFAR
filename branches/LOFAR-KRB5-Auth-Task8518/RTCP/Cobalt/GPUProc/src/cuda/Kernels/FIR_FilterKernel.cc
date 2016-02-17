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
#include <GPUProc/gpu_utils.h>
#include <CoInterface/Align.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <complex>
#include <fstream>

using namespace std;
using boost::lexical_cast;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    string FIR_FilterKernel::theirSourceFile = "FIR_Filter.cu";
    string FIR_FilterKernel::theirFunction = "FIR_filter";

    FIR_FilterKernel::Parameters::Parameters(const Parset& ps, unsigned nrSTABs, bool inputIsStationData, unsigned nrSubbands, unsigned nrChannels, float scaleFactor, const std::string &name) :
      Kernel::Parameters(name),
      nrSTABs(nrSTABs),
      nrBitsPerSample(ps.settings.nrBitsPerSample),

      nrChannels(nrChannels),
      nrSamplesPerChannel(ps.settings.blockSize / nrChannels),

      nrSubbands(nrSubbands),
      scaleFactor(scaleFactor),
      inputIsStationData(inputIsStationData)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.FIR_FilterKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_FIR_FilterKernel.dat") % 
            ps.settings.observationID);

    }

    const unsigned FIR_FilterKernel::Parameters::nrTaps;

    unsigned FIR_FilterKernel::Parameters::nrSamplesPerSubband() const
    {
      return nrChannels * nrSamplesPerChannel;
    }

    unsigned FIR_FilterKernel::Parameters::nrBytesPerComplexSample() const
    {
      return inputIsStationData
               ? 2 * nrBitsPerSample / 8
               : sizeof(std::complex<float>);
    }

    unsigned FIR_FilterKernel::Parameters::nrHistorySamples() const
    {
      return (nrTaps - 1) * nrChannels;
    }

    size_t FIR_FilterKernel::Parameters::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case FIR_FilterKernel::INPUT_DATA: 
        return
          (size_t) nrSamplesPerSubband() *
            nrSTABs * NR_POLARIZATIONS * 
            nrBytesPerComplexSample();
      case FIR_FilterKernel::OUTPUT_DATA:
        return
          (size_t) nrSamplesPerSubband() * nrSTABs * 
            NR_POLARIZATIONS * sizeof(std::complex<float>);
      case FIR_FilterKernel::FILTER_WEIGHTS:
        return 
          (size_t) nrChannels * nrTaps *
            sizeof(float);
      case FIR_FilterKernel::HISTORY_DATA:
        // History is split over 2 bytes in 4-bit mode, to avoid unnecessary packing/unpacking
        return
          (size_t) nrSubbands *
            nrHistorySamples() * nrSTABs * 
            NR_POLARIZATIONS * (nrBitsPerSample == 4 ? 2U : nrBytesPerComplexSample());
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    FIR_FilterKernel::FIR_FilterKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      CompiledKernel(stream, gpu::Function(module, theirFunction), buffers, params),
      params(params),
      filterWeights(stream.getContext(), params.bufferSize(FILTER_WEIGHTS)),
      historySamples(stream.getContext(), params.bufferSize(HISTORY_DATA)),
      historyFlags(boost::extents[params.nrSubbands][params.nrSTABs])
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, filterWeights);
      setArg(3, historySamples);

      unsigned totalNrThreads = params.nrChannels * NR_POLARIZATIONS * 2;
      unsigned nrPasses = ceilDiv(totalNrThreads, maxThreadsPerBlock);

      setEnqueueWorkSizes( gpu::Grid(totalNrThreads, params.nrSTABs),
                           gpu::Block(totalNrThreads / nrPasses, 1) );

      unsigned nrSamples = 
        params.nrSTABs * params.nrChannels * 
        NR_POLARIZATIONS;

      nrOperations = 
        (size_t) nrSamples * params.nrSamplesPerChannel * params.nrTaps * 2 * 2;

      nrBytesRead = 
        (size_t) nrSamples * (params.nrTaps - 1 + params.nrSamplesPerChannel) * 
          params.nrBytesPerComplexSample();

      nrBytesWritten = 
        (size_t) nrSamples * params.nrSamplesPerChannel * sizeof(std::complex<float>);

      // Note that these constant weights are now (unnecessarily) stored on the
      // device for every workqueue. A single copy per device could be used, but
      // first verify that the device platform still allows workqueue overlap.
      FilterBank filterBank(true, params.nrTaps, 
                            params.nrChannels, KAISER);
      filterBank.negateWeights();
      filterBank.scaleWeights(params.scaleFactor);

      gpu::HostMemory firWeights(stream.getContext(), filterWeights.size());
      std::memcpy(firWeights.get<void>(), filterBank.getWeights().origin(),
                  firWeights.size());
      stream.writeBuffer(filterWeights, firWeights, true);

      // start with all history samples flagged
      for (size_t n = 0; n < historyFlags.num_elements(); ++n)
        historyFlags.origin()[n].include(0, params.nrHistorySamples());

      // set all history samples to 0, to prevent adding uninitialised data
      // to the stream
      historySamples.set(0);
    }

    void FIR_FilterKernel::enqueue(const BlockID &blockId,
                                   unsigned subbandIdx)
    {
      setArg(4, subbandIdx);
      Kernel::enqueue(blockId);
    }

    void FIR_FilterKernel::prefixHistoryFlags(MultiDimArray<SparseSet<unsigned>, 1> &inputFlags, unsigned subbandIdx) {
      for (unsigned stationIdx = 0; stationIdx < params.nrSTABs; ++stationIdx) {
        // shift sample flags to the right to make room for the history flags
        inputFlags[stationIdx] += params.nrHistorySamples();

        // add the history flags.
        inputFlags[stationIdx] |= historyFlags[subbandIdx][stationIdx];

        // Save the new history flags for the next block.
        // Note that the nrSamples is the number of samples
        // WITHOUT history samples, but we've also just shifted everything
        // by nrHistorySamples.
        historyFlags[subbandIdx][stationIdx] =
          inputFlags[stationIdx].subset(params.nrSamplesPerSubband(), params.nrSamplesPerSubband() + params.nrHistorySamples());

        // Shift the flags to index 0
        historyFlags[subbandIdx][stationIdx] -= params.nrSamplesPerSubband();
      }
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> CompileDefinitions
    KernelFactory<FIR_FilterKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_STABS"] = 
        lexical_cast<string>(itsParameters.nrSTABs);
      defs["NR_BITS_PER_SAMPLE"] =
        lexical_cast<string>(itsParameters.nrBitsPerSample);

      defs["NR_CHANNELS"] = lexical_cast<string>(itsParameters.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      defs["NR_TAPS"] = 
        lexical_cast<string>(itsParameters.nrTaps);
      defs["NR_SUBBANDS"] = 
        lexical_cast<string>(itsParameters.nrSubbands);

      if (itsParameters.inputIsStationData)
        defs["INPUT_IS_STATIONDATA"] = "1";

      return defs;
    }
  }
}

