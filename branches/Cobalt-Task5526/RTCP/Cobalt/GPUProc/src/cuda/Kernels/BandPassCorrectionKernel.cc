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

#include <GPUProc/global_defines.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/BandPass.h>
#include <CoInterface/BlockID.h>
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
      Kernel::Parameters(ps),
      nrBitsPerSample(ps.settings.nrBitsPerSample),
      nrBytesPerComplexSample(ps.nrBytesPerComplexSample()),
      nrSAPs(ps.settings.SAPs.size()),
      nrDelayCompensationChannels(ps.settings.beamFormer.nrDelayCompensationChannels),
      nrHighResolutionChannels(ps.settings.beamFormer.nrHighResolutionChannels),
      correctBandPass(ps.settings.corrections.bandPass)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.BandPassCorrectionKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_BandPassCorrectionKernel.dat") % 
            ps.settings.observationID );
    }

    BandPassCorrectionKernel::BandPassCorrectionKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, buffers.bandPassCorrectionWeights);

      const unsigned nrChannels2 = params.nrHighResolutionChannels / params.nrDelayCompensationChannels;

      // The cu kernel requires a square for the (x,y) block dimensions.
      //
      // Since the block is connected to (x,y) = (nrChannels2,nrSamples) work division,
      // we need smaller blocks than 16x16 if nrChannels2 is too small.
      const unsigned xyDim = std::min(nrChannels2, 16U);
      
      setEnqueueWorkSizes( gpu::Grid(params.nrSamplesPerChannel,
                                     params.nrDelayCompensationChannels,
                                     nrChannels2),
                                     gpu::Block(xyDim, 1, xyDim));

      size_t nrSamples = params.nrStations * params.nrSamplesPerChannel *
                         params.nrHighResolutionChannels * NR_POLARIZATIONS;
      nrOperations = nrSamples ;
      nrBytesRead = nrBytesWritten = nrSamples * sizeof(std::complex<float>);

      gpu::HostMemory bpWeights(stream.getContext(), buffers.bandPassCorrectionWeights.size());
      BandPass::computeCorrectionFactors(bpWeights.get<float>(),
                                         params.nrHighResolutionChannels,
                                         1.0 / params.nrHighResolutionChannels);
      stream.writeBuffer(buffers.bandPassCorrectionWeights, bpWeights, true);
    }


    //--------  Template specializations for KernelFactory  --------//

    template<> size_t 
    KernelFactory<BandPassCorrectionKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case BandPassCorrectionKernel::INPUT_DATA: 
        return 
            (size_t) itsParameters.nrStations * NR_POLARIZATIONS * 
            itsParameters.nrSamplesPerChannel *
            itsParameters.nrHighResolutionChannels *
            sizeof(std::complex<float>);
      case BandPassCorrectionKernel::OUTPUT_DATA:
        return
            (size_t) itsParameters.nrStations * NR_POLARIZATIONS * 
            itsParameters.nrSamplesPerChannel *
            itsParameters.nrHighResolutionChannels *
            sizeof(std::complex<float>);
      case BandPassCorrectionKernel::BAND_PASS_CORRECTION_WEIGHTS:
        return
            (size_t) itsParameters.nrHighResolutionChannels * sizeof(float);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    template<> CompileDefinitions
    KernelFactory<BandPassCorrectionKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);
      defs["NR_BITS_PER_SAMPLE"] =
        lexical_cast<string>(itsParameters.nrBitsPerSample);
      defs["NR_CHANNELS_1"] =
        lexical_cast<string>(itsParameters.nrDelayCompensationChannels);
      defs["NR_CHANNELS_2"] =
        lexical_cast<string>(itsParameters.nrHighResolutionChannels /
                             itsParameters.nrDelayCompensationChannels);
      if (itsParameters.correctBandPass)
        defs["DO_BANDPASS_CORRECTION"] = "1";
      return defs;
    }
  }
}
