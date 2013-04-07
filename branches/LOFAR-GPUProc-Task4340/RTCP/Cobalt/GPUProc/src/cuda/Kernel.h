//# Kernel.h
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

#ifndef LOFAR_GPUPROC_KERNEL_H
#define LOFAR_GPUPROC_KERNEL_H

#include <cuda.h>

#include <CoInterface/Parset.h>

#include <PerformanceCounter.h>
//#include "opencl-incl.h"
#include "cuwrapper.h"

namespace LOFAR
{
  namespace Cobalt
  {
    class Kernel : public cl::Kernel
    {
    public:
      //Kernel(const Parset &ps, cl::Program &program, const char *name);
      Kernel(const Parset &ps, cu::Module& module, const char *name); // TODO: name -> std::string

      void enqueue(cl::CommandQueue &queue, PerformanceCounter &counter);

    protected:
//      cl::Event event;
      cudaEvent_t event;
      const Parset &ps;
//      cl::NDRange globalWorkSize, localWorkSize;
      dim3 globalWorkSize, localWorkSize;
      size_t nrOperations, nrBytesRead, nrBytesWritten;
    };
  }
}

#endif

