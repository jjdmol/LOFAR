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

#include <GPUProc/gpu_utils.h>

// #include <cstdlib>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/fcntl.h>
// #include <unistd.h>
// #include <cstring>
// #include <cerrno>
#include <iostream>
// #include <sstream>
// #include <set>

// #include <Common/SystemUtil.h>
// #include <Common/SystemCallException.h>
// #include <Stream/FileStream.h>

// #include <GPUProc/global_defines.h>

#define BUILD_MAX_LOG_SIZE	4095

namespace LOFAR
{
  namespace Cobalt
  {
    using namespace std;

    gpu::Module createModule(const gpu::Context &context, 
                             const string &srcFilename,
                             const string &ptx)
    {
      /*
       * JIT compilation options.
       * Note: need to pass a void* with option vals. Preferably, do not alloc
       * dyn (mem leaks on exc).
       * Instead, use local vars for small variables and vector<char> xxx;
       * passing &xxx[0] for output c-strings.
       */
      gpu::Module::optionmap_t options;

#if 0
      size_t maxRegs = 63; // TODO: write this up
      options.push_back(CU_JIT_MAX_REGISTERS);
      optionValues.push_back(&maxRegs);

      size_t thrPerBlk = 256; // input and output val
      options.push_back(CU_JIT_THREADS_PER_BLOCK);
      optionValues.push_back(&thrPerBlk); // can be read back
#endif

      // input and output var for JIT compiler
      size_t infoLogSize  = BUILD_MAX_LOG_SIZE + 1;
      // idem (hence not the a single var or const)
      size_t errorLogSize = BUILD_MAX_LOG_SIZE + 1;

      vector<char> infoLog(infoLogSize);
      options[CU_JIT_INFO_LOG_BUFFER] = &infoLog[0];
      options[CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES] = 
        reinterpret_cast<void*>(infoLogSize);

      vector<char> errorLog(errorLogSize);
      options[CU_JIT_ERROR_LOG_BUFFER] = &errorLog[0];
      options[CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES] = 
        reinterpret_cast<void*>(errorLogSize);

      float &jitWallTime = reinterpret_cast<float&>(options[CU_JIT_WALL_TIME]);

#if 0
      size_t optLvl = 4; // 0-4, default 4
      options[CU_JIT_OPTIMIZATION_LEVEL] = reinterpret_cast<void*>(optLvl);
#endif

#if 0
      // NOTE: There is no need to specify a target. NVCC will use the best one
      // available based on the PTX and the Context.
      size_t jitTarget = target;
      options[CU_JIT_TARGET] = reinterpret_cast<void*>(jitTarget);
#endif

#if 0
      size_t fallback = CU_PREFER_PTX;
      options[CU_JIT_FALLBACK_STRATEGY] = reinterpret_cast<void*>(fallback);
#endif
      try {
        gpu::Module module(context, ptx.c_str(), options);
        // TODO: check what the ptx compiler prints. Don't print bogus. See if
        // infoLogSize indeed is set to 0 if all cool.
        // TODO: maybe retry if buffer len exhausted, esp for errors
        if (infoLogSize > infoLog.size()) {
          // zero-term log and guard against bogus JIT opt val output
          infoLogSize = infoLog.size();
        }
        infoLog[infoLogSize - 1] = '\0';
        cout << "Build info for '" << srcFilename 
             << "' (build time: " << jitWallTime 
             << " ms):" << endl << &infoLog[0] << endl;

        return module;
      } catch (gpu::CUDAException& exc) {
        if (errorLogSize > errorLog.size()) { // idem
          errorLogSize = errorLog.size();
        }
        errorLog[errorLogSize - 1] = '\0';
        cerr << "Build errors for '" << srcFilename 
             << "' (build time: " << jitWallTime 
             << " ms):" << endl << &errorLog[0] << endl;
        throw;
      }
    }

  } // namespace Cobalt
} // namespace LOFAR

