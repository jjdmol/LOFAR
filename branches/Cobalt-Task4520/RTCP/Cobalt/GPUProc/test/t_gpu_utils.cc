//# tSSH.cc
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

#ifdef USE_CUDA

#include <Common/LofarLogger.h>
#include <GPUProc/gpu_utils.h>
#include <UnitTest++.h>
#include <fstream>
#include <cstdio>

using namespace std;
using namespace LOFAR::Cobalt;

#ifdef USE_CUDA
const char* srcFile("t_gpu_utils.cu");
#elif USE_OPENCL
const char* srcFile("t_gpu_utils.cl");
#else
#error "Either USE_CUDA or USE_OPENCL must be defined"
#endif

struct Fixture
{
  Fixture() {
    ofstream ofs(srcFile);
    if (!ofs) throw runtime_error("Failed to create file: " + string(srcFile));
    ofs << "#if defined FOO && FOO != 42\n"
        << "#error FOO != 42\n"
        << "#endif\n"
#ifdef USE_CUDA
        << "__global__ void dummy(void) {}\n"
#elif USE_OPENCL
        << "__kernel void dummy(__global void) {}\n"
#endif
        << endl;
  }
  ~Fixture() {
    remove(srcFile);
  }
};

TEST_FIXTURE(Fixture, CreatePtx)
{
  createPTX(srcFile);
}

TEST_FIXTURE(Fixture, CreatePtxExtraFlag)
{
  CompileFlags flags;
  flags.insert("--source-in-ptx");
  createPTX(srcFile, flags);
}

TEST_FIXTURE(Fixture, CreatePtxWrongExtraFlag)
{
  CompileFlags flags;
  flags.insert("--yaddayadda");
  CHECK_THROW(createPTX(srcFile, flags), GPUProcException);
}

TEST_FIXTURE(Fixture, CreatePtxExtraDef)
{
  CompileDefinitions defs;
  defs["FOO"] = "42";
  createPTX(srcFile, defaultCompileFlags(), defs);
}

TEST_FIXTURE(Fixture, CreatePtxWrongExtraDef)
{
  CompileDefinitions defs;
  defs["FOO"] = "24";
  CHECK_THROW(createPTX(srcFile, defaultCompileFlags(), defs), 
              GPUProcException);
}

TEST_FIXTURE(Fixture, CreateModule)
{
  gpu::Device device(gpu::Platform().devices()[0]);
  createModule(gpu::Context(device), srcFile, createPTX(srcFile));
}

TEST_FIXTURE(Fixture, CreateModuleHighestArch)
{
  // Highest known architecture is 3.5. 
  // Only perform this test if we do NOT have a device with that capability.
  gpu::Device device(gpu::Platform().devices()[0]);
  if (device.getComputeCapabilityMajor() == 3 && 
      device.getComputeCapabilityMinor() >= 5) return;
  CHECK_THROW(createModule(gpu::Context(device),
                           srcFile, 
                           createPTX(srcFile, 
                                     defaultCompileFlags(), 
                                     defaultCompileDefinitions(), 
                                     vector<gpu::Device>())),
              gpu::GPUException);
}

int main()
{
  INIT_LOGGER("t_gpu_utils");
  return UnitTest::RunAllTests() > 0;
}

#else

#include <iostream>

int main()
{
  std::cout << "The GPU wrapper classes are not yet available for OpenCL.\n"
            << "Test skipped." << std::endl;
  return 0;
}

#endif
