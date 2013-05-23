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
#include <boost/format.hpp>

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
    using boost::format;

    namespace {
      // Return the highest compute target supported by the given device
      CUjit_target computeTarget(const gpu::Device &device)
      {
        unsigned major = device.getComputeCapabilityMajor();
        unsigned minor = device.getComputeCapabilityMinor();

        switch (major) {
          case 0:
            return CU_TARGET_COMPUTE_10;

          case 1:
            switch (minor) {
              case 0:
                return CU_TARGET_COMPUTE_10;
              case 1:
                return CU_TARGET_COMPUTE_11;
              case 2:
                return CU_TARGET_COMPUTE_12;
              case 3:
                return CU_TARGET_COMPUTE_13;
              default:
                return CU_TARGET_COMPUTE_13;
            }

          case 2:
            switch (minor) {
              case 0:
                return CU_TARGET_COMPUTE_20;
              case 1:
                return CU_TARGET_COMPUTE_21;
              default:
                return CU_TARGET_COMPUTE_21;
            }

          case 3:
            if (minor < 5) {
              return CU_TARGET_COMPUTE_30;
            } else {
              return CU_TARGET_COMPUTE_35;
            }

          default:
            return CU_TARGET_COMPUTE_35;
        }
      }

      // Return the highest compute target supported by all the given devices
      CUjit_target computeTarget(const std::vector<gpu::Device> &devices)
      {
        if (devices.empty())
          return CU_TARGET_COMPUTE_35;

        CUjit_target minTarget = CU_TARGET_COMPUTE_35;

        for (std::vector<gpu::Device>::const_iterator i = devices.begin(); i != devices.end(); ++i) {
          CUjit_target target = computeTarget(*i);

          if (target < minTarget)
            minTarget = target;
        }

        return minTarget;
      }

      string get_virtarch(CUjit_target target)
      {
        switch (target) {
        default:
          return "";

        case CU_TARGET_COMPUTE_10:
          return "cmpute_10";

        case CU_TARGET_COMPUTE_11:
          return "compute_11";

        case CU_TARGET_COMPUTE_12:
          return "compute_12";

        case CU_TARGET_COMPUTE_13:
          return "compute_13";

        case CU_TARGET_COMPUTE_20:
        case CU_TARGET_COMPUTE_21:
          return "compute_20";

        case CU_TARGET_COMPUTE_30:
        case CU_TARGET_COMPUTE_35:
          return "compute_30";
        }
      }

      string get_gpuarch(CUjit_target target)
      {
        switch (target) {
        default:
          return "";

        case CU_TARGET_COMPUTE_10:
          return "sm_10";

        case CU_TARGET_COMPUTE_11:
          return "sm_11";

        case CU_TARGET_COMPUTE_12:
          return "sm_12";

        case CU_TARGET_COMPUTE_13:
          return "sm_13";

        case CU_TARGET_COMPUTE_20:
        case CU_TARGET_COMPUTE_21:
          return "sm_20";

        case CU_TARGET_COMPUTE_30:
        case CU_TARGET_COMPUTE_35:
          return "sm_30";
        }
      }
    }

    gpu::Module createProgram(const std::vector<gpu::Device> &devices, const std::string &srcFilename, 
      CudaRuntimeCompiler::flags_type flags, CudaRuntimeCompiler::definitions_type definitions )
    {
      // Target the oldest architecture of the given devices
      CUjit_target target = computeTarget(devices);

      // Add the derived target to our flags -- for now, we only compile for
      // the oldest target.
      flags.insert(str(format("gpu-code %s") % get_gpuarch(target)));
      flags.insert(str(format("gpu-architecture %s") % get_virtarch(target)));

      // Create PTX
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
#endif

      unsigned int jitTarget = target;
      options.push_back(CU_JIT_TARGET);
      optionValues.push_back(reinterpret_cast<void*>(jitTarget)); // cast the value itself to a void*!

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

