//# gpu_wrapper.tcc: CUDA-specific wrapper classes for GPU types.
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

#ifndef LOFAR_GPUPROC_CUDA_GPU_WRAPPER_TCC
#define LOFAR_GPUPROC_CUDA_GPU_WRAPPER_TCC

// \file
// Template implementation of CUDA-specific wrapper classes for GPU types.

namespace LOFAR
{
  namespace Cobalt
  {
    namespace gpu
    {
        template <typename T>
        T * HostMemory::get() const
        {
          return static_cast<T *>(getPtr());
        }

        template <typename T>
        void Function::setParameter(size_t index, const T &val)
        {
          setParameter(index, static_cast<const void *>(&val));
        }

    } // namespace gpu

  } // namespace Cobalt

} // namespace LOFAR

#endif

