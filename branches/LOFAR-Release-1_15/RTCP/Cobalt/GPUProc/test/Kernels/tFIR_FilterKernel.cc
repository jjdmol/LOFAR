//# tFIR_FilterKernel.cc: test FIR_FilterKernel class
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
#include <GPUProc/Kernels/FIR_FilterKernel.h>
#include <CoInterface/Parset.h>
#include <Common/lofar_iostream.h>
#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;

struct TestFixture
{
  TestFixture() : ps("tFIR_FilterKernel.in_parset") {}
  ~TestFixture() {}
  Parset ps;
};

TEST_FIXTURE(TestFixture, InputData)
{
  CHECK_EQUAL(size_t(197568),
              FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::INPUT_DATA));
}

TEST_FIXTURE(TestFixture, OutputData)
{
  CHECK_EQUAL(size_t(786432),
              FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::OUTPUT_DATA));
}

TEST_FIXTURE(TestFixture, FilterWeights)
{
  CHECK_EQUAL(size_t(1024),
              FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::FILTER_WEIGHTS));
}

TEST_FIXTURE(TestFixture, MustThrow)
{
  CHECK_THROW(FIR_FilterKernel::bufferSize(ps, FIR_FilterKernel::BufferType(3)),
              GPUProcException);
}

int main()
{
  INIT_LOGGER("tFIR_FilterKernel");
  return UnitTest::RunAllTests() > 0;
}
