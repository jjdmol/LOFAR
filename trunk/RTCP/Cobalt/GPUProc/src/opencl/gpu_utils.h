//# gpu_utils.h
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

#ifndef LOFAR_GPUPROC_OPENCL_GPU_UTILS_H
#define LOFAR_GPUPROC_OPENCL_GPU_UTILS_H

namespace LOFAR
{
  namespace Cobalt
  {

    std::string errorMessage(cl_int error);

    void createContext(cl::Context &, std::vector<cl::Device> &);

    cl::Program createProgram(cl::Context &, std::vector<cl::Device> &,
                              const char *sources, const char *args);


    namespace gpu_utils
    {
      // The sole purpose of this function is to extract detailed error
      // information if a cl::Error was thrown. Since we want the complete
      // backtrace, we cannot simply try-catch in main(), because that would
      // unwind the stack. The only option we have is to use our own terminate
      // handler.
      void terminate();
    }

  } // namespace Cobalt
} // namespace LOFAR

#endif

