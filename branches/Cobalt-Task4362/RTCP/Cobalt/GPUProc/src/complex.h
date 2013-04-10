//# complex.h: Support for complex numbers on GPUs.
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

// \file
// Support for complex numbers on GPUs.

#ifndef LOFAR_GPUPROC_COMPLEX_H
#define LOFAR_GPUPROC_COMPLEX_H

// #if defined (USE_CUDA) && defined (USE_OPENCL)
// # error "Either CUDA or OpenCL must be enabled, not both"
// #endif

//# Forward declaration
namespace LOFAR {
  namespace Cobalt {
    namespace gpu { 
      template<typename T> class complex;
      // template<> class complex<float>;
      // template<> class complex<double>;
    }
  }
}

#include <complex>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace gpu
    { 
      // Template to represent complex numbers on a GPU. 
      // The non-specialized version is based on std::complex.
      // Specializations for float and double are included from a
      // language-specific file (CUDA or OpenCL).
      template<typename T> 
      class complex : public std::complex<T>
      {
      public:
        // Default constructor.
        // __host__ __device__ 
        complex(const T& re = T(), const T& im = T())
          : std::complex<T>(re, im) { }

        // Copy constructor.
        template<typename U>
        // __host__ __device__
        complex(const complex<U>& z)
          : std::complex<T>(z.real(), z.imag()) { }
      };

    } // namespace gpu

  } // namespace Cobalt

} // namespace LOFAR

// #if defined (USE_CUDA)
// # include "cuda/complex.h"
// #elif defined (USE_OPENCL)
// # include "opencl/complex.h"
// #endif

#endif
