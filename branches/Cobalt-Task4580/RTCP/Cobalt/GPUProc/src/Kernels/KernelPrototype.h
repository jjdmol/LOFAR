//# KernelPrototype.h
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

#ifndef LOFAR_GPUPROC_KERNELPROTOTYPE_H
#define LOFAR_GPUPROC_KERNELPROTOTYPE_H

#include <string>
#include <CoInterface/Parset.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    // Declaration of a generic prototype class. For each concrete Kernel class
    // (e.g. FFT_Kernel), a specialization must exist.
    template<typename T> class KernelPrototype
    {
    public:
      // Construct a prototype that can be used to create kernels of type \c T.
      // \note This method must be implemented in a member specialization.
      KernelPrototype(const Parset &ps);

      // Create a new Kernel object of type \c T. Each kernel is associated with
      // a context and has an input- and an output buffer.
      // \note This method must be implemented in a member specialization.
      T* create(const gpu::Context &context,
                gpu::DeviceMemory &input,
                gpu::DeviceMemory &output);
    private:
      // PTX code, generated for kernels of type \c T, using information in the
      // Parset that was passed to the constructor.
      std::string itsPtx;
    };

  } // namespace Cobalt

} // namespace LOFAR

#endif
