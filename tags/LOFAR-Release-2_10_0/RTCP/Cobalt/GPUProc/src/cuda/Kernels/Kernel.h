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

#ifndef LOFAR_GPUPROC_CUDA_KERNEL_H
#define LOFAR_GPUPROC_CUDA_KERNEL_H

#include <string>
#include <iosfwd>
#include <cuda.h>

#include <GPUProc/gpu_wrapper.h>
#include <GPUProc/gpu_utils.h>
#include <GPUProc/PerformanceCounter.h>

namespace LOFAR
{
  namespace Cobalt
  {
    //# Forward declarations
    struct BlockID;

    /*
     * A wrapper for a generic kernel that can be executed on a GPU, and transforms
     * data from an input buffer to an output buffer.
     */
    class Kernel
    {
    public:
      // Parameters that must be passed to the constructor of this Kernel class.
      // TODO: more at constructor passed immediates can be turned into defines
      // (blockDim/gridDim too if enforced fixed (consider conditional define)
      // or drop opt)
      struct Parameters
      {
        Parameters(const std::string &name);

        std::string name;

        bool dumpBuffers;
        std::string dumpFilePattern;
      };

      enum BufferType
      {
        INPUT_DATA,
        OUTPUT_DATA
      };

      // Buffers that must be passed to the constructor of this Kernel class.
      struct Buffers
      {
        Buffers(const gpu::DeviceMemory& in, 
                const gpu::DeviceMemory& out) :
          input(in), output(out)
        {}
        gpu::DeviceMemory input;
        gpu::DeviceMemory output;
      };

      void enqueue(const BlockID &blockId);

      // Warning: user has to make sure the Kernel is not running!
      RunningStatistics getStats() { return itsCounter.getStats(); }

    protected:
      // Construct a kernel.
      Kernel(const gpu::Stream& stream,
             const Buffers &buffers,
             const Parameters &params);

      // Explicit destructor, because the implicitly generated one is public.
      virtual ~Kernel();

      // Performance counter for work done by this kernel
      PerformanceCounter itsCounter;

      size_t nrOperations, nrBytesRead, nrBytesWritten;

      // Launch the actual kernel
      virtual void launch() const = 0;

      // The GPU Stream associated with this kernel.
      gpu::Stream itsStream;

      // Keep a local (reference counted) copy of the buffers we're using
      Buffers itsBuffers;

      // The parameters as given to the constructor.
      Parameters itsParameters;

    private:
      // Dump output buffer of a this kernel to disk. Use \a blockId to
      // distinguish between the different blocks and subbands.
      // \attention This method is for debugging purposes only, as it has a
      // severe impact on performance.
      void dumpBuffers(const BlockID &blockId) const;
    };

    /*
     * A Kernel that is actually a CUDA JIT compiled function.
     */
    class CompiledKernel : public Kernel, public gpu::Function
    {
    protected:
      // Construct a kernel.
      CompiledKernel(
             const gpu::Stream& stream,
             const gpu::Function& function,
             const Buffers &buffers,
             const Parameters &params);

      // Explicit destructor, because the implicitly generated one is public.
      virtual ~CompiledKernel();

      void launch() const;

      // Set the passed execution configuration if supported on the hardware
      // in the stream for this kernel.
      // If not supported and NULL was passed in errorStrings, an exc is thrown.
      // If not supported and errorsStrings is valid, an error string is written
      // to the errorStrings pointer.
      void setEnqueueWorkSizes(gpu::Grid globalWorkSize,
                               gpu::Block localWorkSize,
                               std::string* errorStrings = NULL);

      // Requires call to setEnqueueWorkSizes() first to get meaningful result.
      // Idem for cache and shared memory configuration in the context.
      unsigned getNrBlocksPerMultiProc(unsigned dynSharedMemBytes = 0) const;

      // "The multiprocessor occupancy is the ratio of active warps to the
      // maximum number of warps supported on a multiprocessor of the GPU."
      // This (tries to) mimic what NVIDIA's CUDA_Occupancy_Calculator.xls does.
      //
      // Requires call to setEnqueueWorkSizes() first to get meaningful result.
      // Idem for cache and shared memory configuration in the context.
      // Note: Higher occupancy does not necessarily mean higher performance.
      double predictMultiProcOccupancy(unsigned dynSharedMemBytes = 0) const;

      const unsigned maxThreadsPerBlock;
    private:
      // The grid of blocks dimensions for kernel execution.
      gpu::Grid itsGridDims;

      // The block of threads dimensions for kernel execution.
      gpu::Block itsBlockDims;
    };
  }
}

#endif

