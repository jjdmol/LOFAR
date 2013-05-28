//# Buffers.h
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

#ifndef LOFAR_GPUPROC_CUDA_BUFFERS_H
#define LOFAR_GPUPROC_CUDA_BUFFERS_H

#include <CoInterface/Allocator.h>
#include <CoInterface/MultiDimArray.h>

#include "gpu_wrapper.h"

namespace LOFAR
{
  namespace Cobalt
  {
    // Decide later whether to adapt all WorkQueue users, e.g. to sys/mman.h PROT_*.
    enum cl_mem_flags {CL_MEM_READ_ONLY = 0x1, CL_MEM_WRITE_ONLY, CL_MEM_READ_WRITE};

    // A buffer on the GPU (device), to which CPU (host) buffers can be attached.
    class DeviceBuffer
    {
    public:
      DeviceBuffer( gpu::Stream &queue, cl_mem_flags deviceMemFlags, size_t size )
      :
        buffer(size),
        queue(queue),
        hostMapFlags() // i.e. ignore deviceMemFlags in CUDA
      {
      }

//#if 0
      operator gpu::DeviceMemory & ()
      {
        return buffer;
      }
//#endif

      // Copies data to the GPU
      void hostToDevice(const gpu::HostMemory& hostBuffer, bool synchronous = false)
      {
        queue.writeBuffer(buffer, hostBuffer, synchronous);
      }

      // Copies data from the GPU
      void deviceToHost(gpu::HostMemory& hostBuffer, bool synchronous = false)
      {
        queue.readBuffer(hostBuffer, buffer, synchronous);
      }

      // Allocates a buffer for transfer with the GPU
      // In CUDA, we cannot map a device buffer into host memory.
      // NVIDIA's OpenCL enqueueMapBuffer() copies a device memory region
      // to a new region of host memory.
      // Implement this "manually".
      gpu::HostMemory allocateHostBuffer( size_t size, cl_mem_flags hostBufferFlags = CL_MEM_READ_WRITE )
      {
        ASSERT(size <= buffer.size()); // TODO: remove size arg from function: rely on buffer.size()
        gpu::HostMemory hostMem(/*buffer.size()*/size);
        deviceToHost(hostMem, true);
        hostMapFlags = hostBufferFlags;
        return hostMem;
      }

      // Deallocates a buffer for transfer with the GPU
      // In CUDA, we cannot map a device buffer into host memory.
      // NVIDIA's OpenCL enqueueUnmapBuffer() copies the host memory region
      // back to device memory if the mapping was READ_WRITE.
      // If READ_ONLY, the host memory region is simply discarded.
      // Implement this "manually".
      void deallocateHostBuffer( gpu::HostMemory &hostMemory )
      {
        if (hostMapFlags & CL_MEM_WRITE_ONLY)
        {
          // Copy back to ensure consistency of device memory.
          // Must be synchronous to ensure copying is done before deallocation occurs.
          hostToDevice(hostMemory, true);
        }
        // HostMemory will be deallocated when last hostMemory obj copy goes away.
      }

    private:
      gpu::DeviceMemory buffer;
      gpu::Stream &queue;
      cl_mem_flags hostMapFlags; // TODO: Support >1 mapping (actually, don't and rework interface; RAII (wrt deallocateHostBuffer(), though in ~HostBuffer() below))

      // Can't copy, like with OpenCL cl::Buffer
      DeviceBuffer(const DeviceBuffer &other);
    };

    // A buffer on the CPU (host), attached to a buffer on the GPU (device)
    class HostBuffer
    {
    public:
      HostBuffer( DeviceBuffer &deviceBuffer, size_t size, cl_mem_flags hostBufferFlags = CL_MEM_READ_WRITE )
      :
        hostMemory(deviceBuffer.allocateHostBuffer(size, hostBufferFlags)),
        deviceBuffer(deviceBuffer)
      {
      }

      ~HostBuffer()
      {
        deviceBuffer.deallocateHostBuffer(hostMemory);
      }

      void hostToDevice(bool synchronous = false)
      {
        deviceBuffer.hostToDevice(hostMemory, synchronous);
      }

      void deviceToHost(bool synchronous = false)
      {
        deviceBuffer.deviceToHost(hostMemory, synchronous);
      }

//#if 0
      operator DeviceBuffer& () {
        return deviceBuffer;
      }
//#endif

    private:
      gpu::HostMemory hostMemory;
      DeviceBuffer &deviceBuffer;

      // Copying is expensive (requires allocation),
      // so forbid it to prevent accidental copying.
      HostBuffer(const HostBuffer &other);
    };

    // A MultiDimArray allocated as a HostBuffer
    template <typename T, size_t DIM>
    class MultiArrayHostBuffer : public HostBuffer, public MultiDimArray<T, DIM>
    {
    public:
      template <typename ExtentList>
      MultiArrayHostBuffer(const ExtentList &extents, unsigned int hostBufferFlags, DeviceBuffer &deviceBuffer)
      :
        HostBuffer(deviceBuffer, this->nrElements(extents) * sizeof(T), hostBufferFlags),
        MultiDimArray<T,DIM>(extents, hostMemory.get<T>(), true)
      {
      }

      size_t bytesize() const
      {
        return this->num_elements() * sizeof(T);
      }
    };

    // A 1:1 buffer on CPU and GPU
    template <typename T, size_t DIM>
    class MultiArraySharedBuffer : public DeviceBuffer, public MultiArrayHostBuffer<T, DIM>
    {
    public:
      template <typename ExtentList>
      MultiArraySharedBuffer(const ExtentList &extents, gpu::Stream &queue, cl_mem_flags hostBufferFlags = CL_MEM_READ_WRITE, cl_mem_flags deviceBufferFlags = /* unused */CL_MEM_READ_WRITE)
        :
        DeviceBuffer(queue, deviceBufferFlags, this->nrElements(extents) * sizeof(T)),
        MultiArrayHostBuffer<T, DIM>(extents, hostBufferFlags, *this)
      {
      }

      // Select the desired interface
      using HostBuffer::hostToDevice;
      using HostBuffer::deviceToHost;
//      using DeviceBuffer::operator cl::Buffer&;
    };

#if 0
    // A 1:1 buffer on CPU and GPU
    template <typename T, size_t DIM>
    class MultiArraySharedBuffer : public MultiDimArray<T, DIM>
    {
    public:
      // deviceBufferFlags is unused with CUDA; to correspond to OpenCL
      template <typename ExtentList>
      MultiArraySharedBuffer(const ExtentList &extents, gpu::Stream &queue,
                             cl_mem_flags hostBufferFlags,
                             cl_mem_flags deviceBufferFlags = 0)
        :
        MultiDimArray<T, DIM>(extents, extents, hostBuffer.get<T>(), true),
        deviceBuffer(extents.size()),
        hostBuffer(extends.size(), hostBufferFlags)
      {
      }

      // Copies data to the GPU
      void hostToDevice(bool synchronous = false)
      {
        queue.writeBuffer(deviceBuffer, hostBuffer, synchronous);
      }

      // Copies data from the GPU
      void deviceToHost(bool synchronous = false)
      {
        queue.readBuffer(hostBuffer, deviceBuffer, synchronous);
      }

    private:
      gpu::DeviceMemory deviceBuffer;
      gpu::HostMemory hostBuffer;
    };
#endif

  } // namespace Cobalt
} // namespace LOFAR

#endif

