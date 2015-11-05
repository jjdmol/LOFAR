//# IntToFloatKernel.cc
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

#include <lofar_config.h>

#include "IntToFloatKernel.h"

#include <Common/lofar_complex.h>

#include <GPUProc/global_defines.h>

namespace LOFAR
{
  namespace Cobalt
  {
    IntToFloatKernel::IntToFloatKernel(const Parset &ps, gpu::Stream &queue, gpu::Module &program, gpu::DeviceMemory &devFilteredData, gpu::DeviceMemory &devInputSamples)
      :
      Kernel(ps, program, "intToFloat")
    {
      setArg(0, devFilteredData);
      setArg(1, devInputSamples);

      size_t maxNrThreads;
      //getWorkGroupInfo(queue.getInfo<CL_QUEUE_DEVICE>(), CL_KERNEL_WORK_GROUP_SIZE, &maxNrThreads);
      maxNrThreads = getAttribute(CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK);
      globalWorkSize = gpu::Grid(maxNrThreads, ps.nrStations());
      localWorkSize = gpu::Block(maxNrThreads, 1);

      size_t nrSamples = ps.nrStations() * ps.nrSamplesPerChannel() * ps.nrChannelsPerSubband() * NR_POLARIZATIONS;
      nrOperations = nrSamples * 2;
      nrBytesRead = nrSamples * 2 * ps.nrBitsPerSample() / 8;
      nrBytesWritten = nrSamples * sizeof(std::complex<float>);
    }


  }
}

