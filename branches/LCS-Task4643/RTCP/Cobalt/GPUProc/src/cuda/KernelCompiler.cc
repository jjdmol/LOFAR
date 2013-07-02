//# KernelCompiler.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <GPUProc/KernelCompiler.h>
#include <GPUProc/Kernels/Kernel.h>
#include <boost/format.hpp>
#include <cuda.h>
#include <ostream>

namespace LOFAR
{
  namespace Cobalt
  {
    using namespace std;
    using boost::format;

    // Anonymous namespace; equivalent of C static
    namespace
    {
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

      // Return the lowest compute target supported by all the given devices
      CUjit_target computeTarget(const std::vector<gpu::Device> &devices)
      {
#if CUDA_VERSION >= 5000
        CUjit_target minTarget = CU_TARGET_COMPUTE_35;
#else
        CUjit_target minTarget = CU_TARGET_COMPUTE_30;
#endif

        for (vector<gpu::Device>::const_iterator i = devices.begin(); 
             i != devices.end(); ++i) {
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
        case CU_TARGET_COMPUTE_21: // 21 not allowed for nvcc --gpu-architecture option value
          return "compute_20";

        case CU_TARGET_COMPUTE_30:
          return "compute_30";

#if CUDA_VERSION >= 5000
        case CU_TARGET_COMPUTE_35:
          return "compute_35";
#endif
        }
      }

    } // namespace {anonymous}

#if 0
    bool CompileDefinitions::empty() const
    {
      return defs.empty();
    }

    string& CompileDefinitions::operator[](const string& key)
    {
      return defs[key];
    }
#endif

    const CompileDefinitions& KernelCompiler::defaultDefinitions()
    {
      static CompileDefinitions defs;
      if (defs.empty()) {
        // insert definitions
      }
      return defs;
    }
    
    const CompileFlags& KernelCompiler::defaultFlags()
    {
      static CompileFlags flags;
      if (flags.empty()) {
        flags.insert("-o -");
        flags.insert("-ptx");
        flags.insert("-use_fast_math");
#if CUDA_VERSION >= 5000
        // Set GPU architecture to least capable device available.
        flags.insert(str(format("-arch %s") % 
          get_virtarch(computeTarget(gpu::Platform().devices()))));
#endif
        // Add $LOFARROOT/include to include path, if $LOFARROOT is set.
        const char* lofarroot = getenv("LOFARROOT");
        if (lofarroot) {
          flags.insert(str(format("-I%s/include") % lofarroot));
        }
      }
      return flags;
    }

    ostream& operator<<(ostream& os, const CompileDefinitions& defs)
    {
      CompileDefinitions::const_iterator it;
      for (it = defs.begin(); it != defs.end(); ++it) {
        os << " -D" << it->first;
        if (!it->second.empty()) {
          os << "=" << it->second;
        }
      }
      return os;
    }

    ostream& operator<<(ostream& os, const CompileFlags& flags)
    {
      CompileFlags::const_iterator it;
      for (it = flags.begin(); it != flags.end(); ++it) {
        os << " " << *it;
      }
      return os;
    }

#if 0
    void CompileFlags::add(const string& flag)
    {
      // Make sure an existing flag is erased before we insert, otherwise the
      // insert may fail.
      flags.erase(flag);
      flags.insert(flag);
    }

    bool CompileFlags::empty() const
    {
      return flags.empty();
    }
#endif

    // //---------------------------------------------------------------------//

    // ostream& operator<<(ostream& os, const CompileOptions& opts)
    // {
    //   CompileOptions::const_iterator it;
    //   for (it = opts.begin(); it != opts.end(); ++it) {
    //     os << it->first;
    //     if (!it->second.empty()) {
    //       os << "=" << it->second;
    //     }
    //   }
    //   return os;
    // }


    // PTX store. The key is the complete command line that was passed to the
    // compiler to create PTX code; the PTX code is stored as value.
    typedef map<string, string> PTXStore;

    // The PTX store is implemented as a Meyers singleton. Use a lock to make
    // access thread-safe.
    PTXStore& thePTXStore()
    {
      static PTXStore ptxStore;
      return ptxStore;
    }

    KernelCompiler::KernelCompiler(const Kernel &kernel) 
//      :
//      itsDefinitions(defaultDefinitions()),
//      itsFlags(defaultFlags())
    {
      cout << "########" << __PRETTY_FUNCTION__ << "########" << endl;
    }

    // void KernelCompiler::setOptions(const CompileOptions &options)
    // {
    //   options.insert(itsCompileOptions.begin(), itsCompileOptions.end());
    //   options.swap(itsCompileOptions);
    // }

    string KernelCompiler::compile(string sourceFile,
                                   CompileDefinitions defs,
                                   CompileFlags flags) const
    {
      // Prefix the CUDA kernel filename with $LOFARROOT/share/gpu/kernels
      // if $LOFARROOT is set and if filename is a relative path.
      if (sourceFile[0] != '/') {
        const char* lofarroot = getenv("LOFARROOT");
        string sourceDir = 
          (lofarroot ? str(format("%s/share/gpu/kernels/") % lofarroot) : "");
        sourceFile = sourceDir + sourceFile;
      }
      // Combine the default definitions and flags with the ones passed in.
      defs.insert(defaultDefinitions().begin(), defaultDefinitions().end());
      flags.insert(defaultFlags().begin(), defaultFlags().end());
      
      string ptx;
      ostringstream cmd;
      cmd << "nvcc " << flags << defs << " " << sourceFile;
      LOG_DEBUG_STR("Compiling kernel: " << cmd.str());
      return runNVCC(cmd.str());
    }

  }
}
