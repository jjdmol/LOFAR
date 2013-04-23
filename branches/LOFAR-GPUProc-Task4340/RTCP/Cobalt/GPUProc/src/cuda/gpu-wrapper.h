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
// Uses the "Pimpl" idiom as far as possible. For more info on Pimpl, see
// http://www.boost.org/doc/libs/release/libs/smart_ptr/sp_techniques.html#pimpl

#include <string>
#include <vector>
#include <exception>

// unique_ptr would be enough, but we don't want to require C++11, so use
// shared_ptr instead, which also corresponds better to the OpenCL C++ wrapper.
#include <boost/shared_ptr.hpp>
#include <cuda.h> // ideally, this goes into the .cc, but too much leakage

namespace LOFAR {
namespace Cobalt {
namespace gpu {

  class Error : public std::exception {
    public:
      Error(CUresult result)
      :
        _result(result)
      {
      }

      virtual const char *what() const throw();

      operator CUresult () const
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


#if 0
  inline void init(unsigned flags = 0)
  {
    checkCuCall(cuInit(flags));
  }
#endif

  class Platform { // to correspond to OpenCL
    public:
      Platform(unsigned flags = 0)
      {
        checkCuCall(cuInit(flags));
      }

      ~Platform()
      {
      }

      size_t size()
      {
	int nrDevices;
	checkCuCall(cuDeviceGetCount(&nrDevices));
	return (size_t)nrDevices;
      }
  };


  class Device
  {
  public:
    Device(int ordinal = 0);

#if 0
      static int getCount()
      {
	int nrDevices;
	checkCuCall(cuDeviceGetCount(&nrDevices));
	return nrDevices;
      }
#endif

    std::string getName();

    // template this one to make it similar to getInfo() in OpenCL's C++ wrapper.
    template <CUdevice_attribute attribute>
    int getAttribute() const;

    friend class Context; // Context needs our device (i.e. _impl) to create a context

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


  class Context
  {
  public:
    /*
     * Creates a new CUDA context and associates it with the calling thread.
     * (i.e. setCurrent() implied)
     * Note that a
     */
    Context(Device device, unsigned int flags = 0);

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
      HostMemory(size_t size, int flags = 0)
      {
	checkCuCall(cuMemHostAlloc(&_ptr, size, flags));
      }

      ~HostMemory()
      {
	checkCuCall(cuMemFreeHost(_ptr));
      }

      template <typename T> operator T * ()
      {
	return static_cast<T *>(_ptr);
      }

    private:
      void *_ptr;
  };


  class DeviceMemory
  {
    public:
      DeviceMemory(size_t size)
      {
	checkCuCall(cuMemAlloc(&_ptr, size));
      }

      ~DeviceMemory()
      {
	checkCuCall(cuMemFree(_ptr));
      }

      operator CUdeviceptr ()
      {
	return _ptr;
      }

    private:
      CUdeviceptr _ptr;
  };


  class Module
  {
  public:
    Module(const std::string &file_name);

    Module(const void *data);

    Module(const void *data, std::vector<CUjit_option> &options,
           std::vector<void*> &optionValues);

  private:
    class Impl;
    boost::shared_ptr<Impl> _impl;
  };


#if 0
  class Function
  {
    public:
      Function(Module &module, const std::string &name)
      {
	checkCuCall(cuModuleGetFunction(&_function, module, name.c_str()));
      }

      int getAttribute(CUfunction_attribute attribute)
      {
	int value;
	checkCuCall(cuFuncGetAttribute(&value, attribute, _function));
	return value;
      }

      void setSharedMemConfig(CUsharedconfig config)
      {
	checkCuCall(cuFuncSetSharedMemConfig(_function, config));
      }

      operator CUfunction ()
      {
	return _function;
      }

    private:
      CUfunction _function;
  };


  class Event
  {
    public:
      Event(int flags = CU_EVENT_DEFAULT)
      {
	checkCuCall(cuEventCreate(&_event, flags));
      }

      ~Event()
      {
	checkCuCall(cuEventDestroy(_event));
      }

      float elapsedTime(Event &second)
      {
	float ms;
	checkCuCall(cuEventElapsedTime(&ms, second, _event));
	return ms;
      }

      operator CUevent ()
      {
	return _event;
      }

    private:
      CUevent _event;
  };


  class Stream
  {
    public:
      Stream(int flags = CU_STREAM_DEFAULT)
      {
	checkCuCall(cuStreamCreate(&_stream, flags));
      }

      ~Stream()
      {
	checkCuCall(cuStreamDestroy(_stream));
      }

      void memcpyHtoDAsync(CUdeviceptr devPtr, const void *hostPtr, size_t size)
      {
	checkCuCall(cuMemcpyHtoDAsync(devPtr, hostPtr, size, _stream));
      }

      void memcpyDtoHAsync(void *hostPtr, CUdeviceptr devPtr, size_t size)
      {
	checkCuCall(cuMemcpyDtoHAsync(hostPtr, devPtr, size, _stream));
      }

      void launchKernel(Function &function, unsigned gridX, unsigned gridY, unsigned gridZ, unsigned blockX, unsigned blockY, unsigned blockZ, unsigned sharedMemBytes, const void **parameters)
      {
	checkCuCall(cuLaunchKernel(function, gridX, gridY, gridZ, blockX, blockY, blockZ, sharedMemBytes, _stream, const_cast<void **>(parameters), 0));
      }

      void query()
      {
	checkCuCall(cuStreamQuery(_stream));
      }

      void synchronize()
      {
	checkCuCall(cuStreamSynchronize(_stream));
      }

      void waitEvent(Event &event)
      {
	checkCuCall(cuStreamWaitEvent(_stream, event, 0));
      }

      void record(Event &event)
      {
	checkCuCall(cuEventRecord(event, _stream));
      }

      operator CUstream ()
      {
	return _stream;
      }

    private:
      CUstream _stream;
  };
}
#endif

} // namespace gpu
} // namespace Cobalt
} // namespace LOFAR

#endif

