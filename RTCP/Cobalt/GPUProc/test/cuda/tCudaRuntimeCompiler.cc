//# tCudaRuntimeCompiler.cc
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
#include <GPUProc/cuda/CudaRuntimeCompiler.h>
#include <Common/LofarLogger.h>

using namespace LOFAR::Cobalt;
using namespace std;

int main()
{
  INIT_LOGGER("tCudaRuntimeCompiler");
  // Run a kernel with two different defines, should result in two different kernels.
  // Just run the compiler with two magic numbers and test for the existance of the numbers
  string kernelPath = "tCudaRuntimeCompiler.in_.cu";
  
  // Get an instantiation of the default parameters
  definitions_type definitions = defaultDefinitions();

  // override the default with a magic number
  definitions["NVIDIA_CUDA"] = "123456";

  string ptx1 = compileToPtx(kernelPath, defaultFlags(), definitions);
  definitions["NVIDIA_CUDA"] = "654321";

  string ptx2 = compileToPtx(kernelPath, defaultFlags(), definitions);

  // tests if the magic numbers are inserted into the ptx files
  if ((std::string::npos != ptx1.find("123456")) ||
      (std::string::npos != ptx2.find("654321")))
  {
    return 0;   
  }
  cerr << "did not find the magic numbers in the compiled ptx: 123456 and 654321" << endl;
  cout << ptx1 << ptx2 ;
  return -1;
}

