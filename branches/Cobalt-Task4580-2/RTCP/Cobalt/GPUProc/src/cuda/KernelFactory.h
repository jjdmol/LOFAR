//# KernelFactory.h
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

#ifndef LOFAR_GPUPROC_CUDA_KERNELFACTORY_H
#define LOFAR_GPUPROC_CUDA_KERNELFACTORY_H

#include <string>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    // Declaration of a generic factory class. For each concrete Kernel class
    // (e.g. FIR_FilterKernel), a specialization must exist of the constructor
    // and of the bufferSize() method.
    template<typename T> class KernelFactory
    {
    public:
      typedef typename T::Parameters Parameters;
      typedef typename T::BufferType BufferType;
      typedef typename T::Buffers Buffers;

      KernelFactory(const Parset& ps);

      // Create a new Kernel object of type \c T.
      T* create(const gpu::Stream& stream,
                const Buffers& buffers) const;

      // // Create a new Kernel object of type \c T, using kernel-specific
      // // parameters to instantiate this new object.
      // T* create(const Parameters& params)
      //   { return new T(params); }

      // Return required buffer size for \a bufferType
      size_t bufferSize(BufferType bufferType) const;

    private:
      // PTX code, generated for kernels of type \c T, using information in the
      // Parset that was passed to the constructor.
      std::string itsPTX;

      // Parameters needed to create a Kernel object of type \c T.
      Parameters itsParameters;
    };

  } // namespace Cobalt

} // namespace LOFAR

#endif
