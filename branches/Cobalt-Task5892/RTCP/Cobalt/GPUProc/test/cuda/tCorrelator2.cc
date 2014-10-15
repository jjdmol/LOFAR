//# tCorrelator2.cc: test Correlator CUDA kernel, with full output comparison
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

#include <cstdlib>  // for rand()
#include <string>
#include <iostream>
#include <omp.h>

#include <boost/format.hpp>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>

#include <CoInterface/BlockID.h>
#include <CoInterface/fpequals.h>
#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/MultiDimArrayHostBuffer.h>
#include <GPUProc/Kernels/CorrelatorKernel.h>

using namespace std;
using namespace boost;
using namespace LOFAR::Cobalt::gpu;
using namespace LOFAR::Cobalt;
using namespace LOFAR::TYPES;
using LOFAR::Exception;

Exception::TerminateHandler t(Exception::terminate);

dcomplex correlate( const dcomplex s1, const dcomplex s2 )
{
  return s1 * conj(s2);
}

void runTest(
  Context &ctx, Stream &stream,
  unsigned NR_STATIONS,
  unsigned BLOCKSIZE,
  unsigned NR_INTEGRATIONS,
  unsigned NR_CHANNELS )
{
  LOG_INFO_STR("Testing " << NR_STATIONS << " stations, " << BLOCKSIZE << " samples/block, " << NR_INTEGRATIONS << " subblocks, " << NR_CHANNELS << " channels.");
  const unsigned NR_BASELINES = NR_STATIONS * (NR_STATIONS + 1) / 2;
  const unsigned NR_POLARIZATIONS = 2;
  const unsigned NR_SAMPLES_PER_CHANNEL = BLOCKSIZE / NR_CHANNELS;

  Parset ps;
  ps.add("Observation.VirtualInstrument.stationList", str(format("[%u*CS001]") % NR_STATIONS));
  ps.add("Observation.antennaSet", "LBA_INNER");
  ps.add("Observation.Dataslots.CS001LBA.RSPBoardList", "[0]");
  ps.add("Observation.Dataslots.CS001LBA.DataslotList", "[0]");
  ps.add("Observation.nrBeams", "1");
  ps.add("Observation.Beam[0].subbandList", "[0]");
  ps.add("Observation.DataProducts.Output_Correlated.enabled",   "true");
  ps.add("Observation.DataProducts.Output_Correlated.filenames", "[SB0.MS]");
  ps.add("Observation.DataProducts.Output_Correlated.locations", "[localhost:.]");
  ps.add("Cobalt.blockSize",                         str(format("%u") % BLOCKSIZE));
  ps.add("Cobalt.Correlator.nrChannelsPerSubband",   str(format("%u") % NR_CHANNELS));
  ps.add("Cobalt.Correlator.nrBlocksPerIntegration", "1");
  ps.add("Cobalt.Correlator.nrIntegrationsPerBlock", str(format("%u") % NR_INTEGRATIONS));
  ps.updateSettings();

  CorrelatorKernel::Parameters params(ps);
  KernelFactory<CorrelatorKernel> factory(params);

  // Define dummy Block-ID
  BlockID blockId;

  // Define and fill input with unique values
  DeviceMemory dInput(ctx, factory.bufferSize(CorrelatorKernel::INPUT_DATA));
  MultiDimArrayHostBuffer<fcomplex, 4> hInput(boost::extents[NR_STATIONS][NR_CHANNELS][NR_SAMPLES_PER_CHANNEL][NR_POLARIZATIONS], ctx);

  for (size_t i = 0; i < hInput.num_elements(); ++i)
    hInput.origin()[i] = fcomplex(2 * i, 2 * i + 1);

  // Define output
  DeviceMemory dOutput(ctx, factory.bufferSize(CorrelatorKernel::OUTPUT_DATA));
  MultiDimArrayHostBuffer<fcomplex, 5> hOutput(boost::extents[NR_INTEGRATIONS][NR_BASELINES][NR_CHANNELS][NR_POLARIZATIONS][NR_POLARIZATIONS], ctx);

  // Create kernel
  std::auto_ptr<CorrelatorKernel> kernel(factory.create(stream, dInput, dOutput));

  // Run kernel
  stream.writeBuffer(dInput, hInput, false);
  kernel->enqueue(blockId);
  stream.readBuffer(hOutput, dOutput, true);

  // Verify output
  for (size_t i = 0; i < NR_INTEGRATIONS; ++i) {
    for (size_t s1 = 0, b = 0; s1 < NR_STATIONS; ++s1) {
      for (size_t s2 = 0; s2 <= s1; ++s2, ++b) {
        for (size_t c = 0; c < NR_CHANNELS; ++c) {
          if (NR_CHANNELS > 1 && c == 0)
            continue;

          for (size_t p1 = 0; p1 < NR_POLARIZATIONS; ++p1) {
            for (size_t p2 = 0; p2 < NR_POLARIZATIONS; ++p2) {
              dcomplex sum = 0.0;

              for (size_t t = 0; t < NR_SAMPLES_PER_CHANNEL / NR_INTEGRATIONS; ++t) {
                sum += correlate(
                  hInput[s1][c][i * NR_SAMPLES_PER_CHANNEL / NR_INTEGRATIONS + t][p2],
                  hInput[s2][c][i * NR_SAMPLES_PER_CHANNEL / NR_INTEGRATIONS + t][p1]);
                //LOG_INFO_STR(sum);
              }

              /* We compare relative to the sum we're expecting, NOT relative
               * to the sum itself. This is due to the fact that summing positive
               * and negative numbers results in a smaller sum, but not necessarily
               * in a better precision.
               */

              ASSERTSTR(fpEquals(
                  static_cast<dcomplex>(hOutput[i][b][c][p1][p2]),
                  sum,

                  // sum(1..t) is O(t^2), and correlating is again O(t^2)
                  // precision of float is 2^24
                  pow(BLOCKSIZE * NR_STATIONS, 4)/pow(2,24),
                  true,
                  false),

                  "For "
                    << NR_STATIONS << " stations, "
                    << BLOCKSIZE << " samples/block, "
                    << NR_INTEGRATIONS << " subblocks, "
                    << NR_CHANNELS << " channels: "
                    << "hOutput[" << i << "][" << s1 << ", " << s2 << "][" << c << "][" << p1 << "][" << p2 << "] is "
                    << hOutput[i][b][c][p1][p2]
                    << ", but expected " << sum
                    << " diff: " << (static_cast<dcomplex>(hOutput[i][b][c][p1][p2]) - sum)
                  );
            }
          }
        }
      }
    }
  }
}

void runAllTests( Context &ctx, Stream &stream )
{
  unsigned channels[] = { 1, 16, 64, 256 };

#pragma omp parallel for
  for (unsigned NR_STATIONS = 1; NR_STATIONS <= 5; ++NR_STATIONS)
#pragma omp parallel for
    for (unsigned NR_INTEGRATIONS = 1; NR_INTEGRATIONS <= 3; ++NR_INTEGRATIONS)
#pragma omp parallel for
      for (unsigned c = 0; c < sizeof channels / sizeof channels[0]; ++c) {
        unsigned NR_CHANNELS = channels[c];

        // Blocks can be read either per 16 or per 24. Make sure we read multiple BLOCKS
        // in Correlator.cu.
        runTest(
          ctx, stream,
          NR_STATIONS,
          /* BLOCKSIZE */ NR_INTEGRATIONS * NR_CHANNELS * 16 * 4, // Make sure we don't divide by 24 (by omitting factor 3)
          NR_INTEGRATIONS,
          NR_CHANNELS);

        runTest(
          ctx, stream,
          NR_STATIONS,
          /* BLOCKSIZE */ NR_INTEGRATIONS * NR_CHANNELS * 24 * 3,
          NR_INTEGRATIONS,
          NR_CHANNELS);
      }
}


int main()
{
  omp_set_nested(true);

  INIT_LOGGER("tCorrelator2");
  try 
  {
    gpu::Platform pf;
    cout << "Detected " << pf.size() << " CUDA devices" << endl;
  } 
  catch (gpu::CUDAException& e) 
  {
    cerr << e.what() << endl;
    return 3;
  }

  // Create a stream
  Device device(0);
  Context ctx(device);
  Stream stream(ctx);

  // Run the test
  runAllTests(ctx, stream);

  return 0;
}

