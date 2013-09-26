//# CorrelatorKernel.h
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

#ifndef LOFAR_GPUPROC_CUDA_CORRELATOR_KERNEL_H
#define LOFAR_GPUPROC_CUDA_CORRELATOR_KERNEL_H

#include <GPUProc/Kernels/Kernel.h>
#include <GPUProc/KernelFactory.h>
#include <GPUProc/gpu_wrapper.h>

namespace LOFAR
{
  namespace Cobalt
  {
    class CorrelatorKernel : public Kernel
    {
    public:
      static std::string theirSourceFile;
      static std::string theirFunction;

      struct Parameters : Kernel::Parameters
      {
        Parameters(const Parset& ps);
        std::string dumpFilePattern;
      };

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA
      };

      CorrelatorKernel(const gpu::Stream &stream,
                       const gpu::Module &module,
                       const Buffers &buffers,
                       const Parameters &param);

    private:
      // Dump output buffers of a given kernel to disk. Use \a blockId to
      // distinguish between the different blocks and subbands.
      // \attention This method is for debugging purposes only, as it has a
      // severe impact on performance.
      virtual void dumpBuffers(const BlockID &blockId) const;

      // Keep a local (reference counted) copy of the buffers we're using
      Buffers itsBuffers;

      // Dump file pattern. Contains place holders for Observation ID, subband
      // number, and block number.
      std::string itsDumpFilePattern;
    };

    //# --------  Template specializations for KernelFactory  -------- #//

    template<> size_t
    KernelFactory<CorrelatorKernel>::bufferSize(BufferType bufferType) const;
  }
}

#endif

