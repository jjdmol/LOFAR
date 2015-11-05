//# BeamFormerKernel.cc
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

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "BeamFormerKernel.h"

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <GPUProc/gpu_utils.h>
#include <CoInterface/BlockID.h>
#include <CoInterface/Config.h>

#include <fstream>
#include <algorithm>

using boost::lexical_cast;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    string BeamFormerKernel::theirSourceFile = "BeamFormer.cu";
    string BeamFormerKernel::theirFunction = "beamFormer";

    BeamFormerKernel::Parameters::Parameters(const Parset& ps) :
      nrStations(ps.settings.antennaFields.size()),

      nrChannels(ps.settings.beamFormer.nrHighResolutionChannels),
      nrSamplesPerChannel(ps.nrSamplesPerSubband() / nrChannels),

      nrSAPs(ps.settings.beamFormer.SAPs.size()),
      nrTABs(ps.settings.beamFormer.maxNrCoherentTABsPerSAP()),
      subbandBandwidth(ps.settings.subbandWidth()),
      doFlysEye(ps.settings.beamFormer.doFlysEye)
    {
      dumpBuffers = 
        ps.getBool("Cobalt.Kernels.BeamFormerKernel.dumpOutput", false);
      dumpFilePattern = 
        str(format("L%d_SB%%03d_BL%%03d_BeamFormerKernel.dat") % 
            ps.settings.observationID);
    }

    BeamFormerKernel::BeamFormerKernel(const gpu::Stream& stream,
                                       const gpu::Module& module,
                                       const Buffers& buffers,
                                       const Parameters& params) :
      Kernel(stream, gpu::Function(module, theirFunction), buffers, params)
    {
      setArg(0, buffers.output);
      setArg(1, buffers.input);
      setArg(2, buffers.beamFormerDelays);

      // Beamformer kernel requires 1 channel in the blockDim.z dimension
      setEnqueueWorkSizes(
        gpu::Grid(NR_POLARIZATIONS,
                  std::max(16U, params.nrTABs),  // if < 16 tabs use more to fill out the wave
                  params.nrChannels),
        gpu::Block(NR_POLARIZATIONS,
                   std::max(16U, params.nrTABs),  // if < 16 tabs use more to fill out the wave
                   1));
        // The additional tabs added to fill out the waves are skipped
        // in the kernel file. Additional threads are used to optimize
        // memory access
    }

    void BeamFormerKernel::enqueue(const BlockID &blockId,
                                   double subbandFrequency, unsigned SAP)
    {
      setArg(3, subbandFrequency);
      setArg(4, SAP);
      Kernel::enqueue(blockId);
    }

    //--------  Template specializations for KernelFactory  --------//

    template<> size_t 
    KernelFactory<BeamFormerKernel>::bufferSize(BufferType bufferType) const
    {
      switch (bufferType) {
      case BeamFormerKernel::INPUT_DATA: 
        return
          (size_t) itsParameters.nrChannels *
          itsParameters.nrSamplesPerChannel * NR_POLARIZATIONS *
          itsParameters.nrStations * sizeof(std::complex<float>);
      case BeamFormerKernel::OUTPUT_DATA:
        return
          (size_t) itsParameters.nrChannels * 
          itsParameters.nrSamplesPerChannel * NR_POLARIZATIONS *
          itsParameters.nrTABs * sizeof(std::complex<float>);
      case BeamFormerKernel::BEAM_FORMER_DELAYS:
        return 
          (size_t) itsParameters.nrSAPs * itsParameters.nrStations *
          itsParameters.nrTABs * sizeof(double);
      default:
        THROW(GPUProcException, "Invalid bufferType (" << bufferType << ")");
      }
    }

    
    
    template<> CompileDefinitions
    KernelFactory<BeamFormerKernel>::compileDefinitions() const
    {
      CompileDefinitions defs =
        KernelFactoryBase::compileDefinitions(itsParameters);

      defs["NR_STATIONS"] = lexical_cast<string>(itsParameters.nrStations);

      defs["NR_CHANNELS"] = lexical_cast<string>(itsParameters.nrChannels);
      defs["NR_SAMPLES_PER_CHANNEL"] = 
        lexical_cast<string>(itsParameters.nrSamplesPerChannel);

      defs["NR_SAPS"] =
        lexical_cast<string>(itsParameters.nrSAPs);
      defs["NR_TABS"] =
        lexical_cast<string>(itsParameters.nrTABs);
      defs["SUBBAND_BANDWIDTH"] =
        str(format("%.7f") % itsParameters.subbandBandwidth);
      if (itsParameters.doFlysEye)
        defs["FLYS_EYE"] = "1";

      return defs;
    }
  }
}

