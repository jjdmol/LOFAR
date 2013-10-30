//# tBandPassCorrectionKernel2.cc: test DelayAndBandPassKernel class
//#
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
#include <GPUProc/Kernels/BandPassCorrectionKernel.h>
#include <CoInterface/Parset.h>
#include <Common/lofar_iostream.h>
#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;

struct TestFixture
{
  TestFixture() 
    : 
  ps("tBandPassCorrectionKernel2.in_parset"),
  params(ps)

  {
    BandPassCorrectionKernel::Parameters params(ps);
    params.nrChannels1 = 64;
    params.nrChannels2 = 63;
    factory = new KernelFactory<BandPassCorrectionKernel>(params);
  }


  ~TestFixture() {}

  Parset ps;
  BandPassCorrectionKernel::Parameters params;
  KernelFactory<BandPassCorrectionKernel> * factory;
};

TEST_FIXTURE(TestFixture, InputData)
{
  CHECK_EQUAL(size_t(786432),
              factory->bufferSize(
                BandPassCorrectionKernel::INPUT_DATA));
}

TEST_FIXTURE(TestFixture, OutputData)
{
  CHECK_EQUAL(size_t(786432),
              factory->bufferSize(
                BandPassCorrectionKernel::OUTPUT_DATA));
}

TEST_FIXTURE(TestFixture, Delays)
{
  CHECK_EQUAL(size_t(16),
              factory->bufferSize(
                BandPassCorrectionKernel::DELAYS));
}

TEST_FIXTURE(TestFixture, PhaseOffsets)
{
  CHECK_EQUAL(size_t(16),
              factory->bufferSize(
                BandPassCorrectionKernel::PHASE_OFFSETS));
}

TEST_FIXTURE(TestFixture, BandPassCorrectionWeights)
{
  CHECK_EQUAL(size_t(64),
              factory->bufferSize(
                BandPassCorrectionKernel::BAND_PASS_CORRECTION_WEIGHTS));
}

TEST_FIXTURE(TestFixture, MustThrow)
{
  CHECK_THROW(factory->bufferSize(
                BandPassCorrectionKernel::BufferType(5)),
              GPUProcException);
}

int main()
{
  INIT_LOGGER("tBandPassCorrectionKernel2");
  try {
    gpu::Platform pf;
  } catch (gpu::GPUException&) {
    cerr << "No GPU device(s) found. Skipping tests." << endl;
    return 3;
  }
  return UnitTest::RunAllTests() > 0;
}
