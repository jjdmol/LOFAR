//# Kernel.cc
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

#include <ostream>
#include <boost/format.hpp>

#include <GPUProc/global_defines.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/gpu_utils.h>    // for createModule()

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {
    Kernel::Kernel(const Parset &ps, 
                   const gpu::Context &context,
                   const string& srcFilename,
                   const string& functionName)
      : 
      gpu::Function(
        createModule(context, 
                     srcFilename,
                     createPTX(srcFilename, 
                               compileDefinitions(ps))),
        functionName),
      event(context),
      ps(ps)
    {
    }

    // TODO: Remove
    Kernel::Kernel(const Parset &ps, const gpu::Module& module, const string &name)
      :
      gpu::Function(module, name),
      event(module.getContext()),
      ps(ps)
    {
    }

    void Kernel::enqueue(gpu::Stream &queue/*, PerformanceCounter &counter*/)
    {
      // Unlike OpenCL, no need to check for 0-sized work. CUDA can handle it.
      //if (globalWorkSize.x == 0)
      //  return;

      // TODO: to globalWorkSize in terms of localWorkSize (CUDA) (+ remove assertion): add protected setThreadDim()
      gpu::Block block(localWorkSize);
      assert(globalWorkSize.x % block.x == 0 &&
             globalWorkSize.y % block.y == 0 &&
             globalWorkSize.z % block.z == 0);
      gpu::Grid grid(globalWorkSize.x / block.x,
                     globalWorkSize.y / block.y,
                     globalWorkSize.z / block.z);
      //queue.enqueueNDRangeKernel(*this, gpu::nullDim, globalWorkSize, localWorkSize, 0, &event);
      queue.launchKernel(*this, grid, block);
//      counter.doOperation(event, nrOperations, nrBytesRead, nrBytesWritten);
    }

    // TODO: Remove
    const CompileDefinitions& Kernel::compileDefinitions(const Parset& ps)
    {
      static CompileDefinitions defs;

      using boost::format;

      if (defs.empty()) {
        defs["NVIDIA_CUDA"] = ""; // left-over from OpenCL for Correlator.cl/.cu 
        // TODO: support device specific defs somehow (createPTX() knows about targets, but may be kernel and target specific)
        //if (devices[0].getInfo<CL_DEVICE_NAME>() == "GeForce GTX 680")
        //  defs["USE_FLOAT4_IN_CORRELATOR"] = "";

        // TODO: kernel-specific defs should be specified in the XXXKernel class
        defs["COMPLEX"] = "2";

        defs["NR_BITS_PER_SAMPLE"] = str(format("%u") % ps.nrBitsPerSample());
        defs["SUBBAND_BANDWIDTH"]  = str(format("%.7ff") % ps.subbandBandwidth()); // returns double, so rounding issue?
        defs["NR_SUBBANDS"]        = str(format("%u") % ps.nrSubbands()); // size_t, but %zu not supp
        defs["NR_CHANNELS"]        = str(format("%u") % ps.nrChannelsPerSubband());
        defs["NR_STATIONS"]        = str(format("%u") % ps.nrStations());
        defs["NR_SAMPLES_PER_CHANNEL"] = str(format("%u") % ps.nrSamplesPerChannel());
        defs["NR_SAMPLES_PER_SUBBAND"] = str(format("%u") % ps.nrSamplesPerSubband());
        defs["NR_BEAMS"]           = str(format("%u") % ps.nrBeams());
        defs["NR_TABS"]            = str(format("%u") % ps.nrTABs(0)); // TODO: 0 should be dep on #beams
        defs["NR_COHERENT_STOKES"] = str(format("%u") % ps.nrCoherentStokes()); // size_t
        defs["NR_INCOHERENT_STOKES"] = str(format("%u") % ps.nrIncoherentStokes()); // size_t
        defs["COHERENT_STOKES_TIME_INTEGRATION_FACTOR"]   = str(format("%u") % ps.coherentStokesTimeIntegrationFactor());
        defs["INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR"] = str(format("%u") % ps.incoherentStokesTimeIntegrationFactor());
        defs["NR_POLARIZATIONS"]   = str(format("%u") % NR_POLARIZATIONS);
        defs["NR_TAPS"]            = str(format("%u") % NR_TAPS);
        defs["NR_STATION_FILTER_TAPS"] = str(format("%u") % NR_STATION_FILTER_TAPS);
        if (ps.delayCompensation())
          defs["DELAY_COMPENSATION"] = "";
        if (ps.correctBandPass())
          defs["BANDPASS_CORRECTION"] = "";
        defs["DEDISPERSION_FFT_SIZE"] = str(format("%u") % ps.dedispersionFFTsize()); // size_t
      }

      return defs;
    }

  }
}

