//# tKernel.cc: test creating and running a CUDA kernel from src at runtime
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <cstring>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Parset.h>
#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <GPUProc/global_defines.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main() {
  // Set up gpu environment
  gpu::Platform pf;
  gpu::Device device(0);
  gpu::Context ctx(device);
  vector<string> targets; // unused atm, so can be empty
  string srcFilename("tKernel.in_.cu");
  Parset ps("tKernel.parset.in");

  // Get default parameters for the compiler
  CudaRuntimeCompiler::flags_type flags = CudaRuntimeCompiler::defaultFlags();
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();
  // assign the correct defines
  definitions["NR_BITS_PER_SAMPLE"]= boost::lexical_cast<string>(ps.nrBitsPerSample());
  definitions["SUBBAND_BANDWIDTH"]= boost::lexical_cast<string>(ps.subbandBandwidth()).append("f");
  definitions["NR_SUBBANDS"]= boost::lexical_cast<string>(ps.nrSubbands());
  definitions["NR_CHANNELS"]= boost::lexical_cast<string>(ps.nrChannelsPerSubband());
  definitions["NR_STATIONS"]= boost::lexical_cast<string>(ps.nrStations());
  definitions["NR_SAMPLES_PER_CHANNEL"]= boost::lexical_cast<string>(ps.nrSamplesPerChannel());
  definitions["NR_SAMPLES_PER_SUBBAND"]= boost::lexical_cast<string>(ps.nrSamplesPerSubband());
  definitions["NR_BEAMS"]= boost::lexical_cast<string>(ps.nrBeams());
  definitions["NR_TABS"]= boost::lexical_cast<string>(ps.nrTABs(0)); // TODO: this restricts to the 1st TAB; make more flex
  definitions["NR_COHERENT_STOKES"]= boost::lexical_cast<string>(ps.nrCoherentStokes());
  definitions["NR_INCOHERENT_STOKES"]= boost::lexical_cast<string>(ps.nrIncoherentStokes());
  definitions["COHERENT_STOKES_TIME_INTEGRATION_FACTOR"]= boost::lexical_cast<string>(ps.coherentStokesTimeIntegrationFactor());
  definitions["INCOHERENT_STOKES_TIME_INTEGRATION_FACTOR"]= boost::lexical_cast<string>(ps.incoherentStokesTimeIntegrationFactor());
  definitions["NR_POLARIZATIONS"]= boost::lexical_cast<string>(NR_POLARIZATIONS);
  definitions["NR_TAPS"]= boost::lexical_cast<string>(NR_TAPS);
  definitions["NR_STATION_FILTER_TAPS"]= boost::lexical_cast<string>(NR_STATION_FILTER_TAPS);
  if (ps.delayCompensation()) {
    definitions["DELAY_COMPENSATION"]= "1";
  }
  if (ps.correctBandPass()) {
    definitions["BANDPASS_CORRECTION"]= "1";
  }
  definitions["DEDISPERSION_FFT_SIZE"]= boost::lexical_cast<string>(ps.dedispersionFFTsize());

  gpu::Module module(createProgram(ctx, targets, srcFilename, flags, definitions));
  cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  string entryPointName("testKernel");
  gpu::Function func(module, entryPointName);

  gpu::Stream stream;

  const size_t size = 2 * 1024 * 1024;
  gpu::HostMemory in(size  * sizeof(float));
  gpu::HostMemory out(size * sizeof(float));
  gpu::DeviceMemory d_in(size  * sizeof(float));
  gpu::DeviceMemory d_out(size * sizeof(float));

  // init buffers
  for (size_t i = 0; i < size; i++)
  {
    in.get<float>()[i] = (float)i;
  }
  memset(out.get<void>(), 0, size * sizeof(float));
  const float incVal = 1.5f;

  // kernel args
  func.setArg(0, d_out);
  func.setArg(1, d_in);
  func.setArg(2, size);
  func.setArg(3, incVal);

  // launch args
  gpu::Block block(64); // reasonable for many platforms; assumes it divides size
  gpu::Grid grid(size / block.x);

  stream.writeBuffer(d_in, in); // asynchronous transfer
  stream.launchKernel(func, grid, block);
  stream.readBuffer(out, d_out, true); // synchronous transfer
  cout << "Succesfully executed kernel and read back output data" << endl;

  // check
  size_t nrErrors = 0;
  for (size_t i = 0; i < size; i++)
  {
    if (out.get<float>()[i] != (float)i + incVal)
    {
      nrErrors += 1;
    }
  }

  if (nrErrors > 0)
  {
    cerr << "Error: " << nrErrors << " unexpected output values; printing a few outputs:" << endl;
    for (size_t i = 0; i < (nrErrors < 10 ? nrErrors : 10); i++) // might print only correct output vals...
    {
      cerr << "idx " << i << ": " << out.get<float>()[i] << endl;
    }
    return 1;
  }

  return 0;
}

