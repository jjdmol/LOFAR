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
#include <set>
#include <boost/format.hpp>

#include <Common/SystemUtil.h>
#include <Common/SystemCallException.h>
#include <Stream/FileStream.h>

#include <GPUProc/global_defines.h>
#include "CudaRuntimeCompiler.h"

#define BUILD_MAX_LOG_SIZE	4095

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

#if CUDA_VERSION >= 5000
          case 3:
            if (minor < 5) {
              return CU_TARGET_COMPUTE_30;
            } else {
              return CU_TARGET_COMPUTE_35;
            }

          default:
            return CU_TARGET_COMPUTE_35;
#else
          default:
            return CU_TARGET_COMPUTE_30;
#endif

        }
      }

      // Return the highest compute target supported by all the given devices
      CUjit_target computeTarget(const std::vector<gpu::Device> &devices)
      {
#if CUDA_VERSION >= 5000
        CUjit_target minTarget = CU_TARGET_COMPUTE_35;
#else
        CUjit_target minTarget = CU_TARGET_COMPUTE_30;
#endif

        for (std::vector<gpu::Device>::const_iterator i = devices.begin(); i != devices.end(); ++i) {
          CUjit_target target = computeTarget(*i);

          if (target < minTarget)
            minTarget = target;
        }

        return minTarget;
      }

      // Translate a compute target to a virtual architecture (= the version
      // the .cu file is written in).
      string get_virtarch(CUjit_target target)
      {
        switch (target) {
        default:
          return "compute_unknown";

        case CU_TARGET_COMPUTE_10:
          return "compute_10";

        case CU_TARGET_COMPUTE_11:
          return "compute_11";

        case CU_TARGET_COMPUTE_12:
          return "compute_12";

        case CU_TARGET_COMPUTE_13:
          return "compute_13";

        case CU_TARGET_COMPUTE_20:
          return "compute_20";

        case CU_TARGET_COMPUTE_21:
          return "compute_20"; // 21 not allowed for nvcc --gpu-architecture option value

        case CU_TARGET_COMPUTE_30:
          return "compute_30";

#if CUDA_VERSION >= 5000
        case CU_TARGET_COMPUTE_35:
          return "compute_35";
#endif
        }
      }

      // Translate a compute target to a GPU architecture (= the instruction
      // set supported by the actual GPU).
      string get_gpuarch(CUjit_target target)
      {
        switch (target) {
        default:
          return "sm_unknown";

        case CU_TARGET_COMPUTE_10:
          return "sm_10";

        case CU_TARGET_COMPUTE_11:
          return "sm_11";

        case CU_TARGET_COMPUTE_12:
          return "sm_12";

        case CU_TARGET_COMPUTE_13:
          return "sm_13";

        case CU_TARGET_COMPUTE_20:
          return "sm_20";

        case CU_TARGET_COMPUTE_21:
          return "sm_21";

        case CU_TARGET_COMPUTE_30:
          return "sm_30";

#if CUDA_VERSION >= 5000
        case CU_TARGET_COMPUTE_35:
          return "sm_35";
#endif
        }
      }
    }


    std::string createPTX(const vector<gpu::Device> &devices, const std::string &srcFilename, 
      flags_type &flags, const definitions_type &definitions )
    {
      // The CUDA code is assumed to be written for the architecture of the
      // oldest device.
#if CUDA_VERSION >= 5000
      CUjit_target commonTarget = computeTarget(devices);
      flags.insert(str(format("gpu-architecture %s") % get_virtarch(commonTarget)));
#endif
      //flags.insert(str(format("-I %s") % dirname(__FILE__))); // TODO: refer to src dir (testing) or install dir (installed)

#if 0
      // We'll compile a specific version for each device that has a different
      // architecture.
      set<CUjit_target> allTargets;

      for (vector<gpu::Device>::const_iterator i = devices.begin(); i != devices.end(); ++i) {
        allTargets.insert(computeTarget(*i));
      }

      for (set<CUjit_target>::const_iterator i = allTargets.begin(); i != allTargets.end(); ++i) {
        flags.insert(str(format("gpu-code %s") % get_gpuarch(*i)));
      }
#endif

      // Create and return PTX
      //return compileToPtx(string(dirname(__FILE__)) + "/" + srcFilename, flags, definitions);

      // Get the contents of the LOFARROOT environment variable
      const char* lofarroot = getenv("LOFARROOT");

      // Add $LOFARROOT/include to include path, if $LOFARROOT is set.
      if (lofarroot) {
        flags.insert(str(format("include-path %s/include") % lofarroot));
      }

      // Prefix the CUDA kernel filename with $LOFARROOT/share/gpu/kernels
      // if $LOFARROOT is set
      std::string srcFileDir = 
        (lofarroot ? str(format("%s/share/gpu/kernels/") % lofarroot) : "");

      return compileToPtx(srcFileDir + srcFilename, flags, definitions);
    }


    gpu::Module createModule(const gpu::Context &context, const std::string &srcFilename, const std::string &ptx)
    {
      /*
       * JIT compilation options.
       * Note: need to pass a void* with option vals. Preferably, do not alloc dyn (mem leaks on exc).
       * Instead, use local vars for small variables and vector<char> xxx; passing &xxx[0] for output c-strings.
       */
      gpu::Module::optionmap_t options;

#if 0
      unsigned int maxRegs = 63; // TODO: write this up
      options.push_back(CU_JIT_MAX_REGISTERS);
      optionValues.push_back(&maxRegs);

      unsigned int thrPerBlk = 256; // input and output val
      options.push_back(CU_JIT_THREADS_PER_BLOCK);
      optionValues.push_back(&thrPerBlk); // can be read back
#endif

      unsigned infoLogSize  = BUILD_MAX_LOG_SIZE + 1; // input and output var for JIT compiler
      unsigned errorLogSize = BUILD_MAX_LOG_SIZE + 1; // idem (hence not the a single var or const)

      vector<char> infoLog(infoLogSize);
      options[CU_JIT_INFO_LOG_BUFFER] = &infoLog[0];
      options[CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES] = &infoLogSize;

      vector<char> errorLog(errorLogSize);
      options[CU_JIT_ERROR_LOG_BUFFER] = &errorLog[0];
      options[CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES] = &errorLogSize;

      float jitWallTime = 0.0f; // output val (init it anyway), in milliseconds
      options[CU_JIT_WALL_TIME] = &jitWallTime;

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

