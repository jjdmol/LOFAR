//# gpu_wrapper.h: CUDA-specific wrapper classes for GPU types.
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

#ifndef LOFAR_GPUPROC_CUDA_GPU_WRAPPER_H
#define LOFAR_GPUPROC_CUDA_GPU_WRAPPER_H

// \file cuda/gpu_wrapper.h
// C++ wrappers for CUDA akin the OpenCL C++ wrappers.
// Uses the "Pimpl" idiom for resource managing classes (i.e. that need to
// control copying having a non-trivial destructor. For more info on Pimpl, see
// http://www.boost.org/doc/libs/release/libs/smart_ptr/sp_techniques.html#pimpl
// Not Pimpl-ed are class Platform, Device, and Function.
// These are also passed by value.

#include <cstddef>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <cuda.h> // ideally, this goes into the .cc, but too much leakage

#include <GPUProc/gpu_wrapper.h> // GPUException

#if CUDA_VERSION < 4020
typedef int CUsharedconfig;
#endif

namespace LOFAR
{
  namespace Cobalt
  {
    namespace gpu
    {

      // Exception class for CUDA errors.
      EXCEPTION_CLASS(CUDAException, GPUException);

      // Return the CUDA error string associated with \a errcode.
      std::string errorMessage(CUresult errcode);


      // Struct representing a CUDA Block, which is similar to the @c dim3 type
      // in the CUDA Runtime API.
      struct Block
      {
        Block(unsigned int x_ = 1, unsigned int y_ = 1, unsigned int z_ = 1);
        unsigned int x;
        unsigned int y;
        unsigned int z;
      };


      // Struct representing a CUDA Grid, which is similar to the @c dim3 type
      // in the CUDA Runtime API.
      struct Grid
      {
        Grid(unsigned int x_ = 1, unsigned int y_ = 1, unsigned int z_ = 1);
        unsigned int x;
        unsigned int y;
        unsigned int z;
      };


      // This class is not strictly needed, because in CUDA there's only one
      // platform, but it hides the CUDA calls and makes it similar to OpenCL.
      class Platform
      {
      public:
        // Initialize the CUDA platform.
        // \param flags must be 0 (at least up till CUDA 5.0).
        Platform(unsigned int flags = 0);

        // Returns the number of devices in the CUDA platform.
        size_t size() const;

        // Returns the name of the CUDA platform. (currently, "NVIDIA CUDA")
        std::string getName() const;
      };

      // Wrap a CUDA Device.
      class Device
      {
      public:
        // Create a device.
        // \param ordinal is the device number; 
        //        valid range: [0, Platform.size()-1]
        Device(int ordinal = 0);

        // Return the name of the device in human readable form.
        std::string getName() const;

        // Return information on a specific \a attribute.
        // \param attribute CUDA device attribute
        int getAttribute(CUdevice_attribute attribute) const;

      private:
        // Context needs access to our \c _device to create a context.
        friend class Context;

        // The CUDA device.
        CUdevice _device;
      };


      // Wrap a CUDA Context. Since this class manages a resource (a CUDA
      // context), it uses the pimpl idiom in combination with a reference
      // counted pointer to make it copyable.
      class Context
      {
      public:
        // Create a new CUDA context and associate it with the calling thread.
        // In other words, \c setCurrent() is implied.
        Context(Device device, unsigned int flags = CU_CTX_SCHED_AUTO);

        // Make this context the current context.
        void setCurrent() const;

        // Returns the device associated to the _current_ context.
        Device getDevice() const;

        // Set the cache configuration of the _current_ context.
        void setCacheConfig(CUfunc_cache config) const;

        // Set the shared memory configuration of the _current_ context.
        void setSharedMemConfig(CUsharedconfig config) const;

      private:
        // Non-copyable implementation class.
        class Impl;

        // Reference counted pointer to the implementation class.
        boost::shared_ptr<Impl> _impl;
      };


      // Wrap CUDA Host Memory. This is the equivalent of a OpenCL Buffer. CUDA
      // distinguishes between between host- and device memory, OpenCL does not.
      class HostMemory
      {
      public:
        // Allocate \a size bytes of host memory.
        // \param size number of bytes to allocate
        // \param flags affect allocation
        // \note To create pinned memory, we need to set
        // \code
        // flags = CU_MEMHOSTALLOC_PORTABLE
        // \endcode
        // \note For input buffers we may consider setting
        // \code
        // flags = CU_MEMHOSTALLOC_PORTABLE | CU_MEMHOSTALLOC_WRITECOMBINED
        // \endcode
        // Please refer to the documentation of the function \c cuMemHostAlloc()
        // in the CUDA Driver API for details.
        HostMemory(size_t size, unsigned int flags = 0);

        // Return a pointer to the actual memory.
        // \warning The returned pointer shall not have a lifetime beyond the
        // lifetime of this object (actually the last copy).
        template <typename T>
        T *get() const;

        // Return the size of this memory block.
        size_t size() const;

      private:
        // Get a void pointer to the actual memory from our Impl class. This
        // method is only used by our templated get() method.
        void* getPtr() const;

        // Non-copyable implementation class.
        class Impl;

        // Reference counted pointer to the implementation class.
        boost::shared_ptr<Impl> _impl;
      };


      // Wrap CUDA Device Memory. This is the equivalent of an OpenCL
      // Buffer. CUDA distinguishes between between host- and device memory,
      // OpenCL does not.
      class DeviceMemory
      {
      public:
        // Allocate \a size bytes of device memory.
        DeviceMemory(size_t size);

        // Return the size of this memory block.
        size_t size() const;

      private:
        // Stream needs access to our device ptr for host-to-device transfer.
        friend class Stream;

        // Non-copyable implementation class.
        class Impl;

        // Reference counted pointer to the implementation class.
        boost::shared_ptr<Impl> _impl;
      };


      // Wrap a CUDA Module. This is the equivalent of a OpenCL Program.
      class Module
      {
      public:
        // Load the module in the file \a fname into the current context. The
        // file should be a \e cubin file or a \e ptx file as output by \c nvcc.
        // \param fname name of a module file
        // \note For details, please refer to the documentation of \c
        // cuModuleLoad in the CUDA Driver API.
        Module(const std::string &fname);

        // Load the module pointed to by \a image into the current context. The
        // pointer may point to a null-terminated string containing \e cubin or
        // \e ptx code.
        // \param image pointer to a module image in memory
        // \note For details, please refer to the documentation of \c
        // cuModuleLoadData in the CUDA Driver API.
        Module(const void *image);

        // Load the module pointed to by \a image into the current context. The
        // pointer may point to a null-terminated string containing \e cubin or
        // \e ptx code.
        // \param image pointer to a module image in memory
        // \param options vector of \c CUjit_option items
        // \param optionValues vector of values associated with \a options
        // \note For details, please refer to the documentation of \c
        // cuModuleLoadDataEx in the CUDA Driver API.
        Module(const void *image,
               std::vector<CUjit_option> &options,
               std::vector<void*> &optionValues);

        // \todo This should return a Function object, not a CUfunction.
        CUfunction getKernelEntryPoint(const char* functionName);

      private:
        // Function needs access to our module to create a function.
        friend class Function;

        // Non-copyable implementation class.
        class Impl;

        // Reference counted pointer to the implementation class.
        boost::shared_ptr<Impl> _impl;
      };

      // Wrap a CUDA Device Function. This is the equivalent of an OpenCL
      // Program.
      class Function
      {
      public:
        // Construct a function object by looking up the function \a name in the
        // module \a module.
        Function(Module &module, const std::string &name);

        // Set kernel immediate argument number \a index to \a val.
        // Not for pointers and memory objects (void *, CUdeviceptr).
        template <typename T>
        void setParameter(size_t index, const T &val);

        // Set kernel device memory object argument number \a index to \a val.
        // For device memory objects (CUdeviceptr) as void *, e.g. from DeviceMemory::get().
        // Not for immediates. No need to use template specialization here.
        void setParameter(size_t index, const void *val);

        // Do not use. To protect from passing pointers other than device memory void *.
        template<typename T>
        void setParameter(size_t index, const T *&val); // intentionally not implemented

        // Return information about a function.
        // \note For details on valid values for \a attribute, please refer to
        // the documentation of cuFuncGetAttribute in the CUDA Driver API.
        int getAttribute(CUfunction_attribute attribute) const;

        // Set the shared memory configuration for a device function.
        // \note For details on valid values for \a config, please refer to the
        // documentation of cuFuncSetSharedMemConfig in the CUDA Driver API.
        void setSharedMemConfig(CUsharedconfig config) const;

      private:
        // Stream needs access to our CUDA function to launch a kernel.
        friend class Stream;

        // CUDA function.
        CUfunction _function;

        // function arguments as set.
        std::vector<void *> _kernelParams;
      };

      // Wrap a CUDA Event. This is the equivalent of an OpenCL Event.
      class Event
      {
      public:
        // Construct a CUDA event. This class manages a resource (a CUDA event)
        // and is therefore implemented using the pimpl idiom, using a reference
        // counted pointer to a non-copyable implementation class.
        // \note For details on valid values for \a flags, please refer to the
        // documentation of cuEventCreate in the CUDA Driver API.
        Event(unsigned int flags = CU_EVENT_DEFAULT);

        // Return the elapsed time in milliseconds between this event and the \a
        // second event.
        float elapsedTime(Event &second) const;

        // Wait until all work preceding this event in the same stream has
        // completed.
        void wait();

      private:
        // Stream needs access to our CUDA event to wait for and record events.
        friend class Stream;

        // Non-copyable implementation class.
        class Impl;

        // Reference counted pointer to the implementation class.
        boost::shared_ptr<Impl> _impl;
      };


      // Wrap a CUDA Stream. This is the equivalent of an OpenCL
      // CommandQueue. This class manages a resource (a CUDA stream) and is
      // therefore implemented using the pimpl idiom, using a reference counted
      // pointer to a non-copyable implementation class.
      class Stream
      {
      public:
        // Create a stream.
        // \param flags must be 0 for CUDA < 5.0
        // \note For details on valid values for \a flags, please refer to the
        // documentation of \c cuStreamCreate in the CUDA Driver API.
        Stream(unsigned int flags = 0);  // named CU_STREAM_DEFAULT (0) since CUDA 5.0

        // Transfer data from host memory \a hostMem to device memory \a devMem.
        // \param devMem Device memory that will be copied to.
        // \param hostMem Host memory that will be copied from.
        // \param synchronous Indicates whether the transfer must be done
        //        synchronously or asynchronously.
        void writeBuffer(DeviceMemory &devMem, const HostMemory &hostMem,
                         bool synchronous = false);

        // Transfer data from device memory \a devMem to host memory \a hostMem.
        // \param hostMem Host memory that will be copied to.
        // \param devMem Device memory that will be copied from.
        // \param synchronous Indicates whether the transfer must be done
        //        synchronously or asynchronously.
        void readBuffer(HostMemory &hostMem, const DeviceMemory &devMem,
                        bool synchronous = false);

        // Launch a CUDA function.
        // \param function object containing the function to launch
        // \param grid Grid size (in terms of threads (not blocks))
        // \param block Block (thread group) size
        // \param sharedMemBytes the amount of shared memory per block that is
        //        dynamically allocated at launch-time
        void launchKernel(const Function &function,
                          const Grid &grid, const Block &block,
                          unsigned sharedMemBytes);

        // Check if all operations on this stream have completed.
        // \return true if all completed, or false otherwise.
        bool query() const;

        // Wait until a this stream's tasks are completed.
        void synchronize() const;

        // Let this stream wait on the event \a event.
        void waitEvent(const Event &event) const;

        // Record the event \a event for this stream.
        void recordEvent(const Event &event);

      private:
        // Non-copyable implementation class.
        class Impl;

        // Reference counted pointer to the implementation class.
        boost::shared_ptr<Impl> _impl;
      };


    } // namespace gpu
  } // namespace Cobalt
} // namespace LOFAR

#include "gpu_wrapper.tcc"

#endif

