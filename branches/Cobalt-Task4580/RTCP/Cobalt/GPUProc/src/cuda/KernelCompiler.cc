//# KernelCompiler.cc
//#
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
#include <GPUProc/global_defines.h>
#include <CoInterface/Exceptions.h>
#include <Common/SystemCallException.h>
#include <Common/LofarLogger.h>
#include "cuda_utils.h"

#include <boost/format.hpp>

#include <cstdlib>
#include <ostream>
#include <sstream>

using namespace std;
using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {
    namespace
    {
      string lofarRoot()
      {
        static const char* env;
        static bool init(false);
        if (!init) {
          env = getenv("LOFARROOT");
        }
        return string(env ? env : "");
      }

      string prefixPath()
      {
        if (lofarRoot().empty()) return ".";
        else return lofarRoot() + "/share/gpu/kernels";
      }

      string includePath()
      {
        if (lofarRoot().empty()) return "include";
        else return lofarRoot() + "/include";
      }

      ostream& operator<<(ostream& os, const KernelCompiler::Definitions& defs)
      {
        KernelCompiler::Definitions::const_iterator it;
        for (it = defs.begin(); it != defs.end(); ++it) {
          os << " -D" << it->first;
          if (!it->second.empty()) {
            os << "=" << it->second;
          }
        }
        return os;
      }

      ostream& operator<<(ostream& os, const KernelCompiler::Flags& flags)
      {
        KernelCompiler::Flags::const_iterator it;
        for (it = flags.begin(); it != flags.end(); ++it) {
          os << " " << *it;
        }
        return os;
      }

    } // namespace {anonymous}


    const KernelCompiler::Definitions& KernelCompiler::defaultDefinitions()
    {
      static KernelCompiler::Definitions defs;
      if (defs.empty()) {
        defs["NR_POLARIZATIONS"] = NR_POLARIZATIONS;
        defs["NR_STATION_FILTER_TAPS"] = NR_STATION_FILTER_TAPS;
        defs["NR_TAPS"] = NR_TAPS;
      }
      return defs;
    }

    const KernelCompiler::Flags& KernelCompiler::defaultFlags()
    {
      static KernelCompiler::Flags flags;
      if (flags.empty()) {
        flags.insert("-o -");
        flags.insert("-ptx");
        flags.insert("-use_fast_math");
        flags.insert(str(format("-I%s") % includePath()));
        // Assume that code is written for the least capable device.
        flags.insert(
          str(format("--gpu-architecture %s") % 
              cuda::get_virtarch(
                cuda::computeTarget(defaultDevices()))));
      }
      return flags;
    }

    const KernelCompiler::Devices& KernelCompiler::defaultDevices()
    {
      static KernelCompiler::Devices devices;
      if (devices.empty()) {
        devices = gpu::Platform().devices();
      }
      return devices;
    }


    KernelCompiler::KernelCompiler(const Definitions& definitions,
                                   const Flags& flags,
                                   const Devices& devices)
      :
      itsDevices(devices),
      itsDefinitions(definitions),
      itsFlags(flags)
    {
      // Add default definitions and flags to the user-supplied ones.
      itsDefinitions.insert(defaultDefinitions().begin(), 
                            defaultDefinitions().end());
      itsFlags.insert(defaultFlags().begin(),
                      defaultFlags().end());
    }

 
    string KernelCompiler::createPTX(string srcFilename) const
    {
      // Prefix the CUDA kernel filename if it's a relative path.
      if (!srcFilename.empty() && srcFilename[0] != '/') {
        srcFilename = prefixPath() + "/" + srcFilename;
      }
      // Perform the actual compilation.
      return doCreatePTX(srcFilename, itsFlags, itsDefinitions);
    }


    string KernelCompiler::doCreatePTX(const std::string& source,
                                       const Flags& flags,
                                       const Definitions& defs) const
    {
      ostringstream oss;
      oss << "nvcc " << source << flags << defs;
      string cmd(oss.str());
      LOG_DEBUG_STR("Starting runtime compilation:\n\t" << cmd);

      string ptx;
      char buffer [1024];
      FILE * stream = popen(cmd.c_str(), "r");

      if (!stream) {
        THROW_SYSCALL("popen");
      }
      while (!feof(stream)) {  // NOTE: We do not get stderr
        if (fgets(buffer, sizeof buffer, stream) != NULL) {
          ptx += buffer;
        }
      }
      if (pclose(stream) || ptx.empty()) {
        THROW(GPUProcException, "Runtime compilation failed!\n\t" << cmd);
      }
      return ptx;

    }

  } // namespace Cobalt

} // namespace LOFAR
