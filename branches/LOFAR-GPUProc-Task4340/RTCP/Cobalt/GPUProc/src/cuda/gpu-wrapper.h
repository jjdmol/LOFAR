//# gpu-wrapper.h
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

// \file
// C++ wrappers for CUDA akin the OpenCL C++ wrappers.
// Uses the "Pimpl" idiom for resource managing classes (i.e. that need to
// control copying having a non-trivial destructor. For more info on Pimpl, see
// http://www.boost.org/doc/libs/release/libs/smart_ptr/sp_techniques.html#pimpl
// Not Pimpl-ed are class Platform, Device, and Function.
// These are also passed by value.

#include <cstddef>
#include <string>
#include <vector>
#include <exception>

#include <boost/shared_ptr.hpp>
#include <cuda.h> // ideally, this goes into the .cc, but too much leakage

namespace LOFAR {
namespace Cobalt {
namespace gpu {

  class Error : public std::exception {
    public:
      Error(CUresult result) : _result(result) { }

      virtual const char *what() const throw();

      operator CUresult() const
      {
        return _result;
      }

    private:
      CUresult _result;
  };

  inline void checkCuCall(CUresult result)
  {
    if (result != CUDA_SUCCESS)
      throw Error(result);
  }


  class Platform {
    public:
      /*
       * Initialize the CUDA platform.
       * This object is not strictly needed, but hide the cu calls
       * and make it similar to OpenCL.
       * flags must be 0 (at least up till CUDA 5.0).
       */
      Platform(unsigned int flags = 0);

      /*
       * Returns the number of devices in the CUDA platform (or throws).
       */
      size_t size() const;
  };


  class Device
  {
  public:
    /*
     * ordinal is the device number. Valid range: [0, Platform.size()-1].
     */
    Device(int ordinal = 0);

    std::string getName() const;

    // template this one to make it similar to getInfo() in OpenCL's C++ wrapper.
    template <CUdevice_attribute attribute>
    int getAttribute() const;

    friend class Context; // Context needs our device (i.e. _device) to create a context

  private:
    CUdevice _device;
  };


  class Context
  {
  public:
    /*
     * Creates a new CUDA context and associates it with the calling thread.
     * (i.e. setCurrent() implied)
     */
    Context(Device device, unsigned int flags = CU_CTX_SCHED_AUTO);

    /*
     * Makes this context the current context.
     */
    void setCurrent() const;

    /*
     * Sets the cache config of the _current_ context.
     */
    void setCacheConfig(CUfunc_cache config) const;

    /*
     * Sets the shared memory config of the _current_ context.
     */
    void setSharedMemConfig(CUsharedconfig config) const;

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


  class HostMemory
  {
  public:
    /*
     * We may want to consider flags = CU_MEMHOSTALLOC_PORTABLE
     * and maybe for input buffers, CU_MEMHOSTALLOC_PORTABLE | CU_MEMHOSTALLOC_WRITECOMBINED
     */
    HostMemory(size_t size, unsigned int flags = 0);

    /*
     * The returned ptr cannot have a lifetime beyond the lifetime of this
     * object (actually the last copy).
     */
    template <typename T> T *get() const;

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


  class DeviceMemory
  {
  public:
    DeviceMemory(size_t size);

    friend class Stream; // Stream needs our device ptr (i.e. _impl) to transfer H2D

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


  class Module
  {
  public:
    Module(const std::string &file_name);

    Module(const void *data);

    Module(const void *data, std::vector<CUjit_option> &options,
           std::vector<void*> &optionValues);

    friend class Function; // Function needs our module (i.e. _impl) to create a function

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


  class Function
  {
  public:
    Function(Module &module, const std::string &name);

    int getAttribute(CUfunction_attribute attribute) const;

    void setSharedMemConfig(CUsharedconfig config) const;

    friend class Stream; // Stream needs our function (i.e. _function) to launch a kernel

  private:
    CUfunction _function;
  };


  class Event
  {
  public:
    Event(unsigned int flags = CU_EVENT_DEFAULT);

    ~Event();

    float elapsedTime(Event &second) const;

    friend class Stream; // Stream needs our event (i.e. _impl) to wait for and record events

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


  class Stream
  {
  public:
    Stream(unsigned int flags = CU_STREAM_DEFAULT);

    ~Stream();

    /*
     * Transfers size bytes from hostMem to devMem asynchronously. (1st arg is dst.)
     */
    void memcpyHtoDAsync(DeviceMemory &devMem, const HostMemory &hostMem, size_t size);

    /*
     * Transfers size bytes from devMem to hostMem asynchronously. (1st arg is dst.)
     */
    void memcpyDtoHAsync(HostMemory &hostMem, const DeviceMemory &devMem, size_t size);

    void launchKernel(Function function, unsigned gridX, unsigned gridY,
                      unsigned gridZ, unsigned blockX, unsigned blockY,
                      unsigned blockZ, unsigned sharedMemBytes,
                      const void **parameters);

    /*
     * Check this stream if all its operations have completed.
     * Returns true if all completed, or false if not yet all completed.
     * Throws on other errors (other is vs not yet all completed).
     */
    bool query() const;

    void synchronize() const;

    void waitEvent(const Event &event) const;

    void recordEvent(const Event &event);

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


} // namespace gpu
} // namespace Cobalt
} // namespace LOFAR

#endif

