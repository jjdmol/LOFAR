//# gpu_utils.cc
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

#include "gpu_utils.h"

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <Common/SystemUtil.h>
#include <Common/SystemCallException.h>
#include <Stream/FileStream.h>

#include <GPUProc/global_defines.h>

#define BUILD_MAX_LOG_SIZE	4095
#include "CudaRuntimeCompiler.h"

namespace LOFAR
{
  namespace Cobalt
  {
    using namespace std;

    gpu::Module createProgram(gpu::Context &context, vector<string> &targets, const string &srcFilename, 
      CudaRuntimeCompiler::flags_type flags, CudaRuntimeCompiler::definitions_type definitions )
    {
      string ptxAsString = CudaRuntimeCompiler::compileToPtx(srcFilename, flags, definitions);

      /*
       * JIT compilation options.
       * Note: need to pass a void* with option vals. Preferably, do not alloc dyn (mem leaks on exc).
       * Instead, use local vars for small variables and vector<char> xxx; passing &xxx[0] for output c-strings.
       */
      vector<CUjit_option> options;
      vector<void*> optionValues;

#if 0
      unsigned int maxRegs = 63; // TODO: write this up
      options.push_back(CU_JIT_MAX_REGISTERS);
      optionValues.push_back(&maxRegs);

      unsigned int thrPerBlk = 256; // input and output val
      options.push_back(CU_JIT_THREADS_PER_BLOCK);
      optionValues.push_back(&thrPerBlk); // can be read back
#endif

      unsigned int infoLogSize  = BUILD_MAX_LOG_SIZE + 1; // input and output var for JIT compiler
      unsigned int errorLogSize = BUILD_MAX_LOG_SIZE + 1; // idem (hence not the a single var or const)

      vector<char> infoLog(infoLogSize);
      options.push_back(CU_JIT_INFO_LOG_BUFFER);
      optionValues.push_back(&infoLog[0]);

      options.push_back(CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES);
      optionValues.push_back(&infoLogSize);

      vector<char> errorLog(errorLogSize);
      options.push_back(CU_JIT_ERROR_LOG_BUFFER);
      optionValues.push_back(&errorLog[0]);

      options.push_back(CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES);
      optionValues.push_back(&errorLogSize);

      float jitWallTime = 0.0f; // output val (init it anyway), in milliseconds
      options.push_back(CU_JIT_WALL_TIME);
      optionValues.push_back(&jitWallTime);

#if 0
      unsigned int optLvl = 4; // 0-4, default 4
      options.push_back(CU_JIT_OPTIMIZATION_LEVEL);
      optionValues.push_back(&optLvl);

      options.push_back(CU_JIT_TARGET_FROM_CUCONTEXT);
      optionValues.push_back(NULL); // no option value, but I suppose the array needs a placeholder
#endif
/*
      CUjit_target_enum target = CU_TARGET_COMPUTE_30; // TODO: determine val from auto-detect or whatever
      options.push_back(CU_JIT_TARGET);
      optionValues.push_back(&target);
*/
#if 0
      CUjit_fallback_enum fallback = CU_PREFER_PTX;
      options.push_back(CU_JIT_FALLBACK_STRATEGY);
      optionValues.push_back(&fallback);
#endif
      try {

        gpu::Module module(ptxAsString.c_str(), options, optionValues);
        // TODO: check what the ptx compiler prints. Don't print bogus. See if infoLogSize indeed is set to 0 if all cool.
        // TODO: maybe retry if buffer len exhausted, esp for errors
        if (infoLogSize > infoLog.size()) { // zero-term log and guard against bogus JIT opt val output
          infoLogSize = infoLog.size();
        }
        infoLog[infoLogSize - 1] = '\0';
        cout << "Build info for '" << srcFilename 
             << "' (build time: " << jitWallTime 
             << " us):" << endl << &infoLog[0] << endl;
        return module;
      } catch (gpu::CUDAException& exc) {
        if (errorLogSize > errorLog.size()) { // idem
          errorLogSize = errorLog.size();
        }
        errorLog[errorLogSize - 1] = '\0';
        cerr << "Build errors for '" << srcFilename 
             << "' (build time: " << jitWallTime 
             << " us):" << endl << &errorLog[0] << endl;
        throw;
      }
    }

  } // namespace Cobalt
} // namespace LOFAR

