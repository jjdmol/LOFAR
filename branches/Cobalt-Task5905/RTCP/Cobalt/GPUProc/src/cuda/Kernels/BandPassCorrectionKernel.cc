//# BandPassCorrectionKernel.cc
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

#include "BandPassCorrectionKernel.h"

#include <GPUProc/gpu_utils.h>
#include <GPUProc/BandPass.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>
#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <fstream>

using boost::lexical_cast;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    string BandPassCorrectionKernel::theirSourceFile = "BandPassCorrection.cu";
    string BandPassCorrectionKernel::theirFunction = "bandPassCorrection";

    BandPassCorrectionKernel::Parameters::Parameters(const Parset& ps) :
      Kernel::Parameters("bandpassCorrection"),
      nrStations(ps.settings.antennaFields.size()),
      nrBitsPerSample(ps.settings.nrBitsPerSample),

      nrDelayCompensationChannels(ps.settings.beamFormer.nrDelayCompensationChannels),
      nrHighResolutionChannels(ps.settings.beamFormer.nrHighResolutionChannels),
      nrSamplesPerChannel(ps.settings.blockSize / nrHighResolutionChannels),

      correctBandPass(ps.settings.corrections.bandPass)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.BandPassCorrectionKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_BandPassCorrectionKernel.dat") % 
            ps.settings.observationID );
    }

    size_t BandPassCorrectionKernel::Parameters::bufferSize(BandPassCorrectionKernel::BufferType bufferType) const
    {
      switch (bufferType) {
      case BandPassCorrectionKernel::INPUT_DATA: 
        return 
            (size_t) nrStations * NR_POLARIZATIONS * 
            nrSamplesPerChannel *
            nrHighResolutionChannels *
            sizeof(std::complex<float>);
      case BandPassCorrectionKernel::OUTPUT_DATA:
        return
            (size_t) nrStations * NR_POLARIZATIONS * 
            nrSamplesPerChannel *
            nrHighResolutionChannels *
            sizeof(std::complex<float>);
      case BandPassCorrectionKernel::BAND_PASS_CORRECTION_WEIGHTS:
        return
            (size_t) nrHighResolutionChannels * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    BandPassCorrectionKernel::BandPassCorrectionKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      CompiledKernel(stream, gpu::Function(module, theirFunction), buffers, params),
      bandPassCorrectionWeights(stream.getContext(), params.bufferSize(BAND_PASS_CORRECTION_WEIGHTS))
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, bandPassCorrectionWeights);

      const unsigned nrChannels2 = params.nrHighResolutionChannels / params.nrDelayCompensationChannels;

      // The cu kernel requires a square for the (x,y) block dimensions.
    
      setEnqueueWorkSizes( gpu::Grid(params.nrSamplesPerChannel,
                                     params.nrDelayCompensationChannels,
                                     nrChannels2),
                                     gpu::Block(16, 16, 1));

      size_t nrSamples = params.nrStations * params.nrSamplesPerChannel *
                         params.nrHighResolutionChannels * NR_POLARIZATIONS;
      nrOperations = nrSamples ;
      nrBytesRead = nrBytesWritten = nrSamples * sizeof(std::complex<float>);

      gpu::HostMemory bpWeights(stream.getContext(), bandPassCorrectionWeights.size());
      BandPass::computeCorrectionFactors(bpWeights.get<float>(),
                                         params.nrHighResolutionChannels,
                                         1.0 / params.nrHighResolutionChannels);
      stream.writeBuffer(bandPassCorrectionWeights, bpWeights, true);
    }


    //--------  Template specializations for KernelFactory  --------//

    template<> CompileDefinitions
    KernelFactory<BandPassCorrectionKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_STATIONS"] = lexical_cast<string>(itsParameters.nrStations);
      defs["NR_BITS_PER_SAMPLE"] =
        lexical_cast<string>(itsParameters.nrBitsPerSample);

      defs["NR_CHANNELS_1"] =
        lexical_cast<string>(itsParameters.nrDelayCompensationChannels);
      defs["NR_CHANNELS_2"] =
        lexical_cast<string>(itsParameters.nrHighResolutionChannels /
                             itsParameters.nrDelayCompensationChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      if (itsParameters.correctBandPass)
        defs["DO_BANDPASS_CORRECTION"] = "1";

      return defs;
    }
  }
}
