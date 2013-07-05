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

#ifndef LOFAR_GPUPROC_CUDA_GPU_UTILS_H
#define LOFAR_GPUPROC_CUDA_GPU_UTILS_H

#include "gpu_wrapper.h"

#include <string>

namespace LOFAR
{
  namespace Cobalt
  {
    // Create a Module from a PTX (string).
    // \par context The context that the Module should be associated with.
    // \par srcFilename Name of the
    gpu::Module createModule(const gpu::Context &context,
                             const std::string &srcFilename, 
                             const std::string &ptx);
  }
}

#endif

