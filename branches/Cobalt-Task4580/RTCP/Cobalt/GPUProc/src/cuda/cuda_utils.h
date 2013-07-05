//# cuda_utils.h: CUDA-specific utilities.
//#
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

#ifndef LOFAR_GPUPROC_CUDA_CUDA_UTILS_H
#define LOFAR_GPUPROC_CUDA_CUDA_UTILS_H

#include <GPUProc/gpu_wrapper.h>

#include <cuda.h>
#include <vector>
#include <string>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace cuda
    {
      // Return the highest compute target supported by the given device
      CUjit_target computeTarget(const gpu::Device &device);

      // Return the highest compute target supported by all the given devices
      CUjit_target computeTarget(const std::vector<gpu::Device> &devices);

      // Translate a compute target to a virtual architecture (= the version
      // the .cu file is written in).
      std::string get_virtarch(CUjit_target target);

      // Translate a compute target to a GPU architecture (= the instruction
      // set supported by the actual GPU).
      std::string get_gpuarch(CUjit_target target);

    } // namespace cuda

  } // namespace Cobalt

} // namespace LOFAR

#endif

