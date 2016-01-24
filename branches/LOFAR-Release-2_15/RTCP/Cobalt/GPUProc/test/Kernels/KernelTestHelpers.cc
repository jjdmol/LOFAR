//# KernelTestHelpers.cc: test Kernels/DelayAndBandPassKernel class
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

#include <CoInterface/Parset.h>
#include <boost/lexical_cast.hpp>

#include "KernelTestHelpers.h"

using namespace std;
using namespace LOFAR::Cobalt;
using namespace boost;

void usage(char const *testName)
{
  cout << "Usage: " << testName << " [options] " << endl;
  cout << "" << endl;
  cout << " Performance measurements: ( no output validation)" << endl;
  cout << " -t nrtabs          Number of tabs to create, default == 127" << endl;
  cout << " -c nrchannels      Number of channels to create after delay compensation, default == 64 (equal to nrDelayCompensationChannels disables cascaded FFT)" << endl;
  cout << " -i IdxGPU          GPU index to run kernel on, default == 0" << endl;
  cout << " -s nrStations      Number of antenna fields to create, default == 47" << endl;
  cout << " -b nrSampleBlocks  Number of (x) * nrchannels samples to create, default = 48 * 64 (-> 196608 total samples)" << endl;
  
  cout << " -q stokesType      Stokes type I XXYY or IQUV, default = IQUV" << endl;
  cout << " -d nrDelayCompensationChannels  Number of delaycompensationchannels default == 64" << endl;
  cout << " -e nrChannelsPerSubband     Channels per subband in the output, default == 16" << endl;
  cout << " -f timeIntegrationFactor    How many samples to combine in output sample, default ==1 " << endl;
  cout << " * The kernels might not actually use all these parameters" << endl;
  cout << "" << endl;
  //cout << "If no arguments are provide the kernel with be tested on output validity" << endl;
  cout << "" << endl;
}


KernelParameters::KernelParameters()
{
  nrTabs = 127;
  nrChannels = 64;
  idxGPU = 0;
  nStation = 47;
  nTimeBlocks = 48 * 64;
  parameterParsed = false;
  stokesType = "IQUV";
  nrDelayCompensationChannels = 64;
  nrChannelsPerSubband = 16;
  timeIntegrationFactor = 1;
}

void KernelParameters::print()
{
  cout << "Used Kernel parameters:" << endl
    << "nrTabs      : " << nrTabs << endl
    << "nrChannels  : " << nrChannels << endl
    << "idxGPU      : " << idxGPU << endl
    << "nStation    : " << nStation << endl
    << "nTimeBlocks : " << nTimeBlocks << endl
    << "stokesType : " << stokesType << endl;
}


void  parseCommandlineParameters(int argc, char *argv[], Parset &ps, KernelParameters &params, const char *testName)
{
  int opt;

  // parse all command-line options
  while ((opt = getopt(argc, argv, "t:c:i:s:b:q:d:e:f:")) != -1)
  {
    switch (opt)
    {
    case 't':
      params.nrTabs = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 'c':
      params.nrChannels = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 'i':
      params.idxGPU = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 's':
      params.nStation = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 'b':
      params.nTimeBlocks = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 'q':
      params.stokesType = optarg;
      params.parameterParsed = true;
      break;

    case 'd':
      params.nrDelayCompensationChannels = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 'e':
      params.nrChannelsPerSubband = atoi(optarg);
      params.parameterParsed = true;
      break;

    case 'f':
      params.timeIntegrationFactor = atoi(optarg);
      params.parameterParsed = true;
      break;

    default:
      usage(testName);
      params.parameterParsed = false;  // Do not exit on no arguments
    }
  }

  // we expect no further arguments
  if (optind != argc) {
    usage(testName);
    exit(1);
  }

  // Create a parset with the correct parameters to run a beamformer kernel
  ps.add("Observation.Beam[0].TiedArrayBeam[0].directionType", "J2000");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].angle1", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].angle2", "0.0");
  ps.add("Observation.Beam[0].TiedArrayBeam[0].coherent", "true");
  ps.add("Observation.Beam[0].angle1", "5.0690771926813865");
  ps.add("Observation.Beam[0].angle2", "0.38194688387907605");
  ps.add("Observation.Beam[0].directionType", "J2000");
  ps.add("Observation.Beam[0].nrTabRings", "0");
  ps.add("Observation.Beam[0].nrTiedArrayBeams", lexical_cast<string>(params.nrTabs));
  ps.add("Observation.Beam[0].subbandList", "[24..28]");
  string filenames = "[";
  filenames.append(lexical_cast<string>(params.nrTabs * params.nStation)).append("*BEAM000.h5]");
  string hosts = "[";
  hosts.append(lexical_cast<string>(params.nrTabs * params.nStation)).append("*localhost:.]");
  ps.add("Observation.DataProducts.Output_CoherentStokes.filenames", filenames);
  ps.add("Observation.DataProducts.Output_CoherentStokes.locations", hosts);
  ps.add("Observation.DataProducts.Output_CoherentStokes.enabled", "true");

  ps.add("Observation.DataProducts.Output_Correlated.enabled", "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[SB0.MS, SB1.MS, SB2.MS, SB3.MS, SB4.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[5 * :.]");
  ps.add("Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband", lexical_cast<string>(params.nrChannelsPerSubband));

  ps.add("Cobalt.BeamFormer.CoherentStokes.subbandsPerFile", "512");
  ps.add("Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor", lexical_cast<string>(params.timeIntegrationFactor));
  ps.add("Cobalt.BeamFormer.CoherentStokes.which", params.stokesType);

  ps.add("Cobalt.BeamFormer.nrDelayCompensationChannels", lexical_cast<string>(params.nrDelayCompensationChannels));
  ps.add("Cobalt.BeamFormer.nrHighResolutionChannels", lexical_cast<string>(params.nrChannels));

  ps.add("Cobalt.blockSize", lexical_cast<string>(params.nTimeBlocks * params.nrChannels));

  string stations = "[";
  stations.append(lexical_cast<string>(params.nStation)).append("*RS106]");
  ps.add("Observation.VirtualInstrument.stationList", stations);
  ps.add("Observation.antennaArray", "LBA");
  ps.add("Observation.antennaSet", "LBA_INNER");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.beamList", "[5 * 0]");
  ps.add("Observation.Dataslots.RS106LBA.DataslotList", "[0..4]");
  ps.add("Observation.Dataslots.RS106LBA.RSPBoardList", "[5 * 0]");
  ps.add("Cobalt.correctBandPass", "true");
  ps.add("Cobalt.delayCompensation", "true");
  ps.updateSettings();
  params.print();
}
