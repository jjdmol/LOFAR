//# createProgram.cc
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

#include "createProgram.h"

#include <sstream>
#include <iomanip>

#include <Common/SystemUtil.h>
#include <GPUProc/global_defines.h>
#include "OpenCL_Support.h"

namespace LOFAR
{
  namespace Cobalt
  {
    cl::Program createProgram(const Parset &ps, cl::Context &context, std::vector<cl::Device> &devices, const char *sources)
    {
      std::stringstream args;
      args << "-cl-fast-relaxed-math";

      std::vector<cl_context_properties> properties;
      context.getInfo(CL_CONTEXT_PROPERTIES, &properties);

      if (cl::Platform((cl_platform_id) properties[1]).getInfo<CL_PLATFORM_NAME>() == "NVIDIA CUDA") {
        args << " -cl-nv-verbose";
        args << " -cl-nv-opt-level=99";
        //args << " -cl-nv-maxrregcount=63";
        args << " -DNVIDIA_CUDA";
      }

      //if (devices[0].getInfo<CL_DEVICE_NAME>() == "GeForce GTX 680")
      //args << " -DUSE_FLOAT4_IN_CORRELATOR";

      args << " -I" << dirname(__FILE__);
      args << " -DNR_BITS_PER_SAMPLE=" << ps.nrBitsPerSample();
      args << " -DSUBBAND_BANDWIDTH=" << std::setprecision(7) << ps.subbandBandwidth() << 'f';
      args << " -DNR_SUBBANDS=" << ps.nrSubbands();
      args << " -DNR_CHANNELS=" << ps.nrChannelsPerSubband();
      args << " -DNR_STATIONS=" << ps.nrStations();
      args << " -DNR_SAMPLES_PER_CHANNEL=" << ps.nrSamplesPerChannel();
      args << " -DNR_SAMPLES_PER_SUBBAND=" << ps.nrSamplesPerSubband();
      args << " -DNR_BEAMS=" << ps.nrBeams();
      args << " -DNR_TABS=" << ps.nrTABs(0);
      args << " -DNR_COHERENT_STOKES=" << ps.nrCoherentStokes();
      args << " -DNR_INCOHERENT_STOKES=" << ps.nrIncoherentStokes();
      args << " -DCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.coherentStokesTimeIntegrationFactor();
      args << " -DINCOHERENT_STOKES_TIME_INTEGRATION_FACTOR=" << ps.incoherentStokesTimeIntegrationFactor();
      args << " -DNR_POLARIZATIONS=" << NR_POLARIZATIONS;
      args << " -DNR_TAPS=" << NR_TAPS;
      args << " -DNR_STATION_FILTER_TAPS=" << NR_STATION_FILTER_TAPS;

      if (ps.delayCompensation())
        args << " -DDELAY_COMPENSATION";

      if (ps.correctBandPass())
        args << " -DBANDPASS_CORRECTION";

      args << " -DDEDISPERSION_FFT_SIZE=" << ps.dedispersionFFTsize();
      return createProgram(context, devices, dirname(__FILE__).append("/").append(sources).c_str(), args.str().c_str());
    }
  }
}

