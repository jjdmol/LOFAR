//# KernelCompiler.h
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

#ifndef LOFAR_GPUPROC_KERNELCOMPILER_H
#define LOFAR_GPUPROC_KERNELCOMPILER_H

#include <GPUProc/gpu_wrapper.h>

#include <string>
#include <map>
#include <set>
#include <vector>
#include <iosfwd>

namespace LOFAR
{
  namespace Cobalt
  {
    class KernelCompiler
    {
    public:
      // Map for storing compile definitions that will be passed to the GPU
      // kernel compiler on the command line. The key is the name of the
      // preprocessor variable, a value should only be assigned if the
      // preprocessor variable needs to have a value.
      struct Definitions : std::map<std::string, std::string>
      {
      };

      // Return default compile definitions
      static const Definitions& defaultDefinitions();

      // Set for storing compile flags that will be passed to the GPU kernel
      // compiler on the command line. Flags generally don't have an associated
      // value; if they do the value should become part of the flag in this set.
      struct Flags : std::set<std::string>
      {
      };

      // Return default compile flags.
      static const Flags& defaultFlags();

      // Return default devices to compile for. The list comprises of the
      // available devices in the current system.
      typedef std::vector<gpu::Device> Devices;
      static const Devices& defaultDevices();

      // Construct a kernel compiler object. This object can be used to compile
      // multiple kernel source to PTX, using the same compile definitions,
      // flags, and target device(s).
      KernelCompiler(const Definitions& definitions = defaultDefinitions(),
                     const Flags& flags = defaultFlags(),
                     const Devices& devices = defaultDevices());

      // Compile \a srcFilename and return the PTX code as string.
      // \par srcFilename Name of the file containing the source code to be
      //      compiled to PTX. If \a srcFilename is a relative path and if the
      //      environment variable \c LOFARROOT is set, then prefix \a
      //      srcFilename with the path \c $LOFARROOT/share/gpu/kernels.
      // \note The argument \a srcFilename is passed by value intentionally,
      //       because it will be modified by this method.
      std::string createPTX(std::string srcFilename) const;

    private:
      // Perform the actual compilation of \a source to PTX. The PTX code will
      // be returned as string.
      std::string doCreatePTX(const std::string& source,
                              const Flags& flags,
                              const Definitions& defs) const;

      Devices itsDevices;
      Definitions itsDefinitions;
      Flags itsFlags;
    };

  } // namespace Cobalt

} // namespace LOFAR

#endif
