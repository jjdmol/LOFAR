//# BeamFormerKernelPerformance.cc
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

//#include <complex>

#include <Common/LofarLogger.h>

#include <boost/shared_ptr.hpp>

#include <GPUProc/gpu_wrapper.h>
//#include <GPUProc/global_defines.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
//#include <CoInterface/BlockID.h>
#include <CoInterface/Parset.h>

//#include "BeamFormerSubbandProcStep.h"
//#include "BeamFormerCoherentStep.h"
#include <GPUProc/SubbandProcs/BeamFormerFactories.h>

#include <iomanip>
#include <iostream>

//beamFormerBuffers = std::auto_ptr<BeamFormerKernel::Buffers>(
//  new BeamFormerKernel::Buffers(*devB, *devA, *devBeamFormerDelays));



using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;


BeamFormerKernel::Parameters
beamFormerParams(const Parset &ps) 
{
  BeamFormerKernel::Parameters params(ps);
  params.nrChannelsPerSubband =
    ps.settings.beamFormer.nrHighResolutionChannels;
  params.nrSamplesPerChannel = ps.nrSamplesPerSubband() /
    ps.settings.beamFormer.nrHighResolutionChannels;

  return params;
}


int main(int argc, char *argv[])
{
  INIT_LOGGER("BeamFormerKernelPerformance");

  // Boiledplate code: create the environment etc;
  gpu::Device device(gpu::Platform().devices()[0]);
  gpu::Context context(device);
  gpu::Stream stream(context);

  // Create a parset with the correct parameters to run a beamformer kernel
  Parset ps;
  ps.add("Observation.Beam[0].TiedArrayBeam[0].directionType","J2000");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].absoluteAngle1", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].absoluteAngle2", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].coherent", "true");
  ps.add("Observation.Beam[0].angle1", "5.0690771926813865");
  ps.add("Observation.Beam[0].angle2", "0.38194688387907605");
  ps.add("Observation.Beam[0].directionType", "J2000");
  ps.add("Observation.Beam[0].nrTabRings", "0");
  ps.add("Observation.Beam[0].nrTiedArrayBeams", "1");

  ps.add("Observation.DataProducts.Output_CoherentStokes.filenames", "[BEAM000.h5]");
  ps.add("Observation.DataProducts.Output_CoherentStokes.locations", "[localhost:.]");

  ps.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", "1");
  ps.add("Cobalt.BeamFormer.CoherentStokes.subbandsPerFile", "512");
  ps.add("Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", "16");
  ps.add("Cobalt.BeamFormer.CoherentStokes.which", "I");

  ps.add("Cobalt.BeamFormer.nrDelayCompensationChannels", "64");
  ps.add("Cobalt.BeamFormer.nrHighResolutionChannels", "64");

  ps.updateSettings();

  // Create a beamformer kernel factory 
  KernelFactory<BeamFormerKernel> beamFormer(ps);
  boost::shared_ptr<gpu::DeviceMemory> a_buffer;

  // Assign a buffers
  a_buffer.reset( new gpu::DeviceMemory( context,
      beamFormer.bufferSize(BeamFormerKernel::INPUT_DATA)));

  a_buffer.reset(new gpu::DeviceMemory(context,
    beamFormer.bufferSize(BeamFormerKernel::OUTPUT_DATA)));

  // Start the kernel with all the needed parameters



  cout << "Hello World!" << endl;
}