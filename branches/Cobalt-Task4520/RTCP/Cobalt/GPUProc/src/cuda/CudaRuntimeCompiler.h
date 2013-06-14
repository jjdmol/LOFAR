//# CudaRuntimeCompiler.h
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

#ifndef LOFAR_GPUPROC_CUDA_CUDA_RUNTIME_COMPILER_H
#define LOFAR_GPUPROC_CUDA_CUDA_RUNTIME_COMPILER_H

#include <cstdio>   // popen, pget
#include <iostream>  
#include <string>
#include <sstream>
#include <iomanip>  // setprecision
#include <map>
#include <set>

#include <CoInterface/Parset.h>
#include <GPUProc/Kernels/CompileDefinitions.h>
#include <GPUProc/gpu_utils.h>
#include "gpu_incl.h"

// Collection of functions needed for runtime compilation of a kernel supplied 
// as a path to a ptx string.
namespace LOFAR
{
  namespace Cobalt
  {

  // // flags
  // typedef std::set<std::string> flags_type;

  // Return the set of default flags for the nvcc compilation of a cuda kernel in Cobalt
  flags_type defaultFlags();
  
  // // Return the set of default definitions for the nvcc compilation of a cuda kernel in Cobalt
  // // DEFINITION=0 results in a definition and cannot for used for undef. Do not insert parameters in this case
  // definitions_type defaultDefinitions();
  
  // Performs a 'system' call of nvcc. Return the stdout of the command
  // on error no stdout is created and an exception is thrown
  std::string runNVCC(const std::string &cmd);
  
  // Create a nvcc command line string based on the input path, a set of flags and a map
  // of definitions. Use this command to call nvcc and compile the file at input path to a ptx file
  // which content is returned as a string
  std::string 
  compileToPtx(const std::string& pathToCuFile, 
               const flags_type& flags = defaultFlags(),
               const CompileDefinitions& definitions = CompileDefinitions());
  
  // overloaded function. Use the path and default flags and definitions to call nvcc
  }
}

#endif

