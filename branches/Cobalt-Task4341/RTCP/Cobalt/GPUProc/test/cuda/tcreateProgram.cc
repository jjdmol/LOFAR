//# tcreateProgram.cc: test CUDA kernel runtime compilation from src file
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

#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Parset.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <GPUProc/global_defines.h>

using namespace std;
using namespace LOFAR::Cobalt;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "Usage: " << argv[0] << " <L12345.parset> <kernel.cu>" << endl;
    return 1;
  }

  // Create the gpu parts needed for running a kernel
  gpu::Platform pf;
  gpu::Device device(0);
  vector<gpu::Device> devices(1, device);
  gpu::Context ctx(device);

  // Open inputs
  Parset ps(argv[1]);
  string srcFilename(argv[2]);

  // Collect inputs from the parste and assign them to CudaRuntimeCompiler
  // input_types.
  CudaRuntimeCompiler::flags_type flags = CudaRuntimeCompiler::defaultFlags();
  CudaRuntimeCompiler::definitions_type definitions = CudaRuntimeCompiler::defaultDefinitions();
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

  string ptx = createPTX(devices, srcFilename, flags, definitions);
  gpu::Module module(createModule(ctx, srcFilename, ptx));
  cout << "Succesfully compiled '" << srcFilename << "'" << endl;

  return 0;
}

