//# tFFT_Kernel.cc: test FFT_Kernel class
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
#include <GPUProc/Kernels/FFT_Kernel.h>
#include <CoInterface/Config.h>
#include <CoInterface/Parset.h>
#include <Common/lofar_iostream.h>
#include <UnitTest++.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;

TEST(InputData)
{
  CHECK_EQUAL(size_t(786432),
              FFT_Kernel::Parameters(16, 49152 * NR_POLARIZATIONS, true).bufferSize(FFT_Kernel::INPUT_DATA));
}

TEST(OutputData)
{
  CHECK_EQUAL(size_t(786432),
              FFT_Kernel::Parameters(16, 49152 * NR_POLARIZATIONS, false).bufferSize(FFT_Kernel::OUTPUT_DATA));
}

TEST(MustThrow)
{
  CHECK_THROW(FFT_Kernel::Parameters(0, 0, true).bufferSize(FFT_Kernel::BufferType(2)),
              GPUProcException);
}

int main()
{
  INIT_LOGGER("tFFT_Kernel");
  return UnitTest::RunAllTests() > 0;
}
