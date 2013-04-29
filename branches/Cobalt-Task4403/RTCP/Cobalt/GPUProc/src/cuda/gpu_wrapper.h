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
//
// Not Pimpl-ed are class Platform, Device, and Function.
// These are also passed by value.

#include <cstddef>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <cuda.h> // ideally, this goes into the .cc, but too much leakage

#include <GPUProc/gpu_wrapper.h>
#include <Common/Exception.h>

namespace LOFAR {
namespace Cobalt {
namespace gpu {

  // Exception class for CUDA errors.
  EXCEPTION_CLASS(CUDAException, GPUException);

  // Return the CUDA error string associated with \a errcode.
  std::string errorMessage(CUresult errcode);

  // This object is not strictly needed, because in CUDA there's only one
  // platform, but it hides the CUDA calls and makes it similar to OpenCL.
  class Platform {
  public:
    // Initialize the CUDA platform.
    // \param flags must be 0 (at least up till CUDA 5.0).
    Platform(unsigned int flags = 0);

    // Returns the number of devices in the CUDA platform.
    size_t size() const;
  };

  // Wrap a CUDA Device.
  class Device
  {
  public:
    // Create a device.
    // \param ordinal is the device number; valid range: [0, Platform.size()-1]
    Device(int ordinal = 0);

    // Return the name of the device in human readable form.
    std::string getName() const;

    // Return information on a specific \a attribute.
    // This method is similar to \c getInfo() in the OpenCL C++ wrapper.
    // \tparam CUdevice_attribute CUDA device attribute type
    template <CUdevice_attribute attribute>
    int getAttribute() const;

  private:
    // Context needs access to our \c _device to create a context.
    friend class Context; 

    // The CUDA device.
    CUdevice _device;
  };


  // Wrap a CUDA Context. Since this class manages a resource (a CUDA context),
  // it uses the pimpl idiom in combination with a reference counted pointer to
  // make it copyable.
  class Context
  {
  public:
    // Create a new CUDA context and associate it with the calling thread.
    // In other words, \c setCurrent() is implied.
    Context(Device device, unsigned int flags = CU_CTX_SCHED_AUTO);

    // Make this context the current context.
    void setCurrent() const;

    // Set the cache configuration of the current context.
    void setCacheConfig(CUfunc_cache config) const;

    // Set the shared memory configuration of the current context.
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
    // Please refer to the documentation of the function \c cuMemHostAlloc() in
    // the CUDA Driver API for details.
    HostMemory(size_t size, unsigned int flags = 0);

    // Return a pointer to the actual memory. 
    // \warning The returned pointer shall not have a lifetime beyond the
    // lifetime of this object (actually the last copy).
    template <typename T> T *get() const;

  private:
    // Non-copyable implementation class.
    class Impl;

    // Reference counted pointer to the implementation class.
    boost::shared_ptr<Impl> _impl;
  };


  // Wrap CUDA Device Memory. This is the equivalent of an OpenCL Buffer. CUDA
  // distinguishes between between host- and device memory, OpenCL does not.
  class DeviceMemory
  {
  public:
    // Alocate \a size bytes of device memory.
    DeviceMemory(size_t size);

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
    // Load the module in the file \a fname into the current context. The file
    // should be a \e cubin file or a \e ptx file as output by \c nvcc.
    // \param fname name of a module file
    // \note For details, please refer to the documentation of \c cuModuleLoad
    // in the CUDA Driver API.
    Module(const std::string &fname);

    // Load the module pointed to by \a image into the current context. The
    // pointer may point to a null-terminated string containing \e cubin or \e
    // ptx code.
    // \param image pointer to a module image in memory
    // \note For details, please refer to the documentation of \c
    // cuModuleLoadData in the CUDA Driver API.
    Module(const void *image);

    // Load the module pointed to by \a image into the current context. The
    // pointer may point to a null-terminated string containing \e cubin or \e
    // ptx code.
    // \param image pointer to a module image in memory
    // \param options vector of \c CUjit_option items
    // \param optionValues vector of values associated with \a options
    // \note For details, please refer to the documentation of \c
    // cuModuleLoadDataEx in the CUDA Driver API.
    Module(const void *image, 
           std::vector<CUjit_option> &options,
           std::vector<void*> &optionValues);

  private:
    // Function needs access to our module to create a function.
    friend class Function; 

    // Non-copyable implementation class.
    class Impl;

    // Reference counted pointer to the implementation class.
    boost::shared_ptr<Impl> _impl;
  };

  // Wrap a CUDA Device Function. This is the equivalent of an OpenCL Program.
  class Function
  {
  public:
    // Construct a function object by looking up the function \a name in the
    // module \a module.
    Function(Module &module, const std::string &name);

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
  };

  // Wrap a CUDA Event. This is the equivalent of an OpenCL Event.
  class Event
  {
  public:
    // Construct a CUDA event. This class manages a resource (a CUDA event) and
    // is therefore implemented using the pimpl idiom, using a reference counted
    // pointer to a non-copyable implementation class.
    // \note For details on valid values for \a flags, please refer to the
    // documentation of cuEventCreate in the CUDA Driver API.
    Event(unsigned int flags = CU_EVENT_DEFAULT);

    // Return the elapsed time in milliseconds between this event and the \a
    // second event.
    float elapsedTime(Event &second) const;

  private:
    // Stream needs access to our CUDA event to wait for and record events.
    friend class Stream;

    // Non-copyable implementation class.
    class Impl;

    // Reference counted pointer to the implementation class.
    boost::shared_ptr<Impl> _impl;
  };


  // Wrap a CUDA Stream. This is the equivalent of an OpenCL CommandQueue. This
  // class manages a resource (a CUDA stream) and is therefore implemented using
  // the pimpl idiom, using a reference counted pointer to a non-copyable
  // implementation class.
  class Stream
  {
  public:
    // Create a stream. 
    // \param flags must be 0 for CUDA < 5.0
    // \note For details on valid values for \a flags, please refer to the
    // documentation of \c cuStreamCreate in the CUDA Driver API.
    Stream(unsigned int flags = 0);

    // Transfers \a size bytes asynchronously from host memory \a hostMem to
    // device memory \a devMem.
    void memcpyHtoDAsync(DeviceMemory &devMem, const HostMemory &hostMem, 
                         size_t size);

    // Transfers \a size bytes asynchronously from device memory \a devMem to
    // host memory \a hostMem asynchronously. 
    void memcpyDtoHAsync(HostMemory &hostMem, const DeviceMemory &devMem, 
                         size_t size);

    // Launch a CUDA function. 
    // \param function object containing the function to launch
    // \param gridX x-coordinate of the grid
    // \param gridY y-coordinate of the grid
    // \param gridZ z-coordinate of the grid
    // \param blockX x-coordinate of the block
    // \param blockY y-coordinate of the block
    // \param blockZ z-coordinate of the block
    // \param sharedMemBytes the amount of shared memory per thread block
    // \param parameters array of pointers to the parameters that must be passed
    //        to the function \a function
    // \todo It's probably better to store the function parameters in the
    // Function object, and define a setParameters() method in Function to set
    // them.
    void launchKernel(const Function &function, 
                      unsigned gridX, unsigned gridY, unsigned gridZ, 
                      unsigned blockX, unsigned blockY, unsigned blockZ,
                      unsigned sharedMemBytes, const void **parameters);

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

#endif

