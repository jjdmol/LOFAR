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

#ifndef LOFAR_GPUPROC_KERNELFACTORY_H
#define LOFAR_GPUPROC_KERNELFACTORY_H

#include <string>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class KernelFactoryBase
    {
    public:
      // Abstract base class of the templated KernelFactory class that can be
      // used to create kernels of type \c T.
      KernelFactoryBase(const Parset &ps);

    private:
      // PTX code, generated for kernels of type \c T, using information in the
      // Parset that was passed to the constructor.
      std::string itsPtx;

      // Private copy of the parset
      Parset ps;
    };

    // Declaration of a generic factory class. For each concrete Kernel class
    // (e.g. FIR_FilterKernel), a class specialization must exist that must
    // inherit from KernelFactoryBase.
    template<typename T> class KernelFactory : public KernelFactoryBase
    {
    public:
      KernelFactory(const Parset& ps) : KernelFactoryBase(ps)
      { }
      // Create a new Kernel object of type \c T.
      T* create(const gpu::Context &context,
                const gpu::DeviceMemory &input,
                const gpu::DeviceMemory &output) const;
      // T* create(const typename T::Parameters& params) {
      //   return new T(params);
      // }

      enum BufferType {};

      // Return required buffer size for \a bufferType
      size_t bufferSize(BufferType bufferType);
    };

  } // namespace Cobalt

} // namespace LOFAR

#endif
