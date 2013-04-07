//# cuwrapper.h
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

#ifndef LOFAR_GPUPROC_CUWRAPPER_H
#define LOFAR_GPUPROC_CUWRAPPER_H

#include <string>
#include <exception>
#include <cuda.h>

namespace LOFAR {
namespace Cobalt {
namespace cu {

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

  inline void checkCudaCall(CUresult result)
  {
    if (result != CUDA_SUCCESS)
      throw Error(result);
  }


  inline void init(unsigned flags = 0)
  {
    checkCudaCall(cuInit(flags));
  }


  class Device {
    public:
      static int getCount()
      {
	int nrDevices;
	checkCudaCall(cuDeviceGetCount(&nrDevices));
	return nrDevices;
      }

      Device(int ordinal)
      {
	checkCudaCall(cuDeviceGet(&_device, ordinal));
      }

      std::string getName() const
      {
	char name[64];
	checkCudaCall(cuDeviceGetName(name, sizeof name, _device));
	return std::string(name);
      }

      template <CUdevice_attribute attribute> int getAttribute() const
      {
	int value;
	checkCudaCall(cuDeviceGetAttribute(&value, attribute, _device));
	return value;
      }

      operator CUdevice ()
      {
	return _device;
      }

    private:
      CUdevice _device;
  };


  class Context
  {
    public:
      Context(int flags, Device device)
      {
	checkCudaCall(cuCtxCreate(&_context, flags, device));
      }

      ~Context()
      {
	checkCudaCall(cuCtxDestroy(_context));
      }

      void setCurrent() const
      {
	checkCudaCall(cuCtxSetCurrent(_context));
      }

      operator CUcontext ()
      {
	return _context;
      }

    private:
      CUcontext _context;
  };


  class HostMemory
  {
    public:
      HostMemory(size_t size, int flags = 0)
      {
	checkCudaCall(cuMemHostAlloc(&_ptr, size, flags));
      }

      ~HostMemory()
      {
	checkCudaCall(cuMemFreeHost(_ptr));
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
	checkCudaCall(cuMemAlloc(&_ptr, size));
      }

      ~DeviceMemory()
      {
	checkCudaCall(cuMemFree(_ptr));
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
      Module(const char *file_name)
      {
	checkCudaCall(cuModuleLoad(&_module, file_name));
      }

      Module(const void *data)
      {
	checkCudaCall(cuModuleLoadData(&_module, data));
      }

      Module(const void *data, std::vector<CUjit_option>& options, std::vector<void*> optionValues) // TODO: get rid of void*
      {
        checkCudaCall(cuModuleLoadDataEx(&_module, data, options.size(), &options[0], &optionValues[0]));
      }

      ~Module()
      {
	checkCudaCall(cuModuleUnload(_module));
      }

      operator CUmodule ()
      {
	return _module;
      }

    private:
      CUmodule _module;
  };


  class Function
  {
    public:
      Function(Module &module, const char *name)
      {
	checkCudaCall(cuModuleGetFunction(&_function, module, name));
      }

      int getAttribute(CUfunction_attribute attribute)
      {
	int value;
	checkCudaCall(cuFuncGetAttribute(&value, attribute, _function));
	return value;
      }

      void setSharedMemConfig(CUsharedconfig config)
      {
	checkCudaCall(cuFuncSetSharedMemConfig(_function, config));
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
	checkCudaCall(cuEventCreate(&_event, flags));
      }

      ~Event()
      {
	checkCudaCall(cuEventDestroy(_event));
      }

      float elapsedTime(Event &second)
      {
	float ms;
	checkCudaCall(cuEventElapsedTime(&ms, second, _event));
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
	checkCudaCall(cuStreamCreate(&_stream, flags));
      }

      ~Stream()
      {
	checkCudaCall(cuStreamDestroy(_stream));
      }

      void memcpyHtoDAsync(CUdeviceptr devPtr, const void *hostPtr, size_t size)
      {
	checkCudaCall(cuMemcpyHtoDAsync(devPtr, hostPtr, size, _stream));
      }

      void memcpyDtoHAsync(void *hostPtr, CUdeviceptr devPtr, size_t size)
      {
	checkCudaCall(cuMemcpyDtoHAsync(hostPtr, devPtr, size, _stream));
      }

      void launchKernel(Function &function, unsigned gridX, unsigned gridY, unsigned gridZ, unsigned blockX, unsigned blockY, unsigned blockZ, unsigned sharedMemBytes, const void **parameters)
      {
	checkCudaCall(cuLaunchKernel(function, gridX, gridY, gridZ, blockX, blockY, blockZ, sharedMemBytes, _stream, const_cast<void **>(parameters), 0));
      }

      void query()
      {
	checkCudaCall(cuStreamQuery(_stream));
      }

      void synchronize()
      {
	checkCudaCall(cuStreamSynchronize(_stream));
      }

      void waitEvent(Event &event)
      {
	checkCudaCall(cuStreamWaitEvent(_stream, event, 0));
      }

      void record(Event &event)
      {
	checkCudaCall(cuEventRecord(event, _stream));
      }

      operator CUstream ()
      {
	return _stream;
      }

    private:
      CUstream _stream;
  };
}

} // namespace cu
} // namespace Cobalt
} // namespace LOFAR

#endif

