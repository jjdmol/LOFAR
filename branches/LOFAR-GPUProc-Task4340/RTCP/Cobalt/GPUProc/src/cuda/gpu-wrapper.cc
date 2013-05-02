//# gpu-wrapper.cc
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

#include <lofar_config.h> // always included in LOFAR .cc, but not used here

#include "gpu-wrapper.h"

#include <boost/noncopyable.hpp>

namespace LOFAR {
namespace Cobalt {
namespace gpu {

using namespace std;

const char *Error::what() const throw()
{
  switch (_result) {
    case CUDA_SUCCESS:
      return "success";
    case CUDA_ERROR_INVALID_VALUE:
      return "invalid value";
    case CUDA_ERROR_OUT_OF_MEMORY:
      return "out of memory";
    case CUDA_ERROR_NOT_INITIALIZED:
      return "not initialized";
    case CUDA_ERROR_DEINITIALIZED:
      return "deinitialized";
    case CUDA_ERROR_PROFILER_DISABLED:
      return "profiler disabled";
    case CUDA_ERROR_PROFILER_NOT_INITIALIZED:
      return "profiler not initialized";
    case CUDA_ERROR_PROFILER_ALREADY_STARTED:
      return "profiler already started";
    case CUDA_ERROR_PROFILER_ALREADY_STOPPED:
      return "profiler already stopped";
    case CUDA_ERROR_NO_DEVICE:
      return "no device";
    case CUDA_ERROR_INVALID_DEVICE:
      return "invalid device";
    case CUDA_ERROR_INVALID_IMAGE:
      return "invalid image";
    case CUDA_ERROR_INVALID_CONTEXT:
      return "invalid context";
    case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:
      return "context already current";
    case CUDA_ERROR_MAP_FAILED:
      return "map failed";
    case CUDA_ERROR_UNMAP_FAILED:
      return "unmap failed";
    case CUDA_ERROR_ARRAY_IS_MAPPED:
      return "array is mapped";
    case CUDA_ERROR_ALREADY_MAPPED:
      return "already mapped";
    case CUDA_ERROR_NO_BINARY_FOR_GPU:
      return "no binary for GPU";
    case CUDA_ERROR_ALREADY_ACQUIRED:
      return "already acquired";
    case CUDA_ERROR_NOT_MAPPED:
      return "not mapped";
    case CUDA_ERROR_NOT_MAPPED_AS_ARRAY:
      return "not mapped as array";
    case CUDA_ERROR_NOT_MAPPED_AS_POINTER:
      return "not mapped as pointer";
    case CUDA_ERROR_ECC_UNCORRECTABLE:
      return "ECC uncorrectable";
    case CUDA_ERROR_UNSUPPORTED_LIMIT:
      return "unsupported limit";
    case CUDA_ERROR_CONTEXT_ALREADY_IN_USE:
      return "context already in use";
    case CUDA_ERROR_INVALID_SOURCE:
      return "invalid source";
    case CUDA_ERROR_FILE_NOT_FOUND:
      return "file not found";
    case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND:
      return "shared object symbol not found";
    case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED:
      return "shared object init failed";
    case CUDA_ERROR_OPERATING_SYSTEM:
      return "operating system";
    case CUDA_ERROR_INVALID_HANDLE:
      return "invalid handle";
    case CUDA_ERROR_NOT_FOUND:
      return "not found";
    case CUDA_ERROR_NOT_READY:
      return "not ready";
    case CUDA_ERROR_LAUNCH_FAILED:
      return "launch failed";
    case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:
      return "launch out of resources";
    case CUDA_ERROR_LAUNCH_TIMEOUT:
      return "launch timeout";
    case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:
      return "launch incompatible texturing";
    case CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED:
      return "peer access already enabled";
    case CUDA_ERROR_PEER_ACCESS_NOT_ENABLED:
      return "peer access not enabled";
    case CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE:
      return "primary context active";
    case CUDA_ERROR_CONTEXT_IS_DESTROYED:
      return "context is destroyed";
    case CUDA_ERROR_UNKNOWN:
      return "unknown";
    default:
      return "unknown error code";
  }
}


  Platform::Platform(unsigned int flags)
  {
    checkCuCall(cuInit(flags));
  }

  size_t Platform::size() const
  {
    int nrDevices;
    checkCuCall(cuDeviceGetCount(&nrDevices));
    return (size_t)nrDevices;
  }


  Device::Device(int ordinal)
  {
    checkCuCall(cuDeviceGet(&_device, ordinal));
  }

  string Device::getName() const
  {
    // NV ref is not crystal clear on returned str len. Better be safe.
    const size_t max_name_len = 255;
    char name[max_name_len + 1];
    checkCuCall(cuDeviceGetName(name, max_name_len, _device));
    return string(name);
  }

  int Device::getAttribute(CUdevice_attribute attribute) const
  {
    int value;
    checkCuCall(cuDeviceGetAttribute(&value, attribute, _device));
    return value;
  }


  class Context::Impl : boost::noncopyable
  {
  public:
    Impl(CUdevice device, unsigned int flags)
    {
      checkCuCall(cuCtxCreate(&_context, flags, device));
    }

    ~Impl()
    {
      checkCuCall(cuCtxDestroy(_context));
    }

    void setCurrent() const
    {
      checkCuCall(cuCtxSetCurrent(_context));
    }

    void setCacheConfig(CUfunc_cache config) const
    {
      checkCuCall(cuCtxSetCacheConfig(config));
    }

    void setSharedMemConfig(CUsharedconfig config) const
    {
      checkCuCall(cuCtxSetSharedMemConfig(config));
    }

  private:
    CUcontext _context;
  };

  Context::Context(Device device, unsigned int flags)
    : _impl(new Impl(device._device, flags)) { }

  void Context::setCurrent() const
  {
    _impl->setCurrent();
  }

  void Context::setCacheConfig(CUfunc_cache config) const
  {
    _impl->setCacheConfig(config);
  }

  void Context::setSharedMemConfig(CUsharedconfig config) const
  {
    _impl->setSharedMemConfig(config);
  }


  class HostMemory::Impl : boost::noncopyable
  {
  public:
    Impl(size_t size, unsigned int flags)
    {
      checkCuCall(cuMemHostAlloc(&_ptr, size, flags));
    }

    ~Impl()
    {
      checkCuCall(cuMemFreeHost(_ptr));
    }

    void *get() const
    {
      return _ptr;
    }

  private:
    void *_ptr;
  };

  HostMemory::HostMemory(size_t size, unsigned int flags)
    : _impl(new Impl(size, flags)) { }

  void *HostMemory::doGet() const
  {
    return _impl->get();
  }


  class DeviceMemory::Impl : boost::noncopyable
  {
  public:
    Impl(size_t size)
    {
      checkCuCall(cuMemAlloc(&_ptr, size));
    }

    ~Impl()
    {
      checkCuCall(cuMemFree(_ptr));
    }

    void *get() const
    {
      return reinterpret_cast<void *>(_ptr);
    }

  //private: // Stream needs it to do transfers
    CUdeviceptr _ptr;
  };

  DeviceMemory::DeviceMemory(size_t size)
    : _impl(new Impl(size)) { }

  void *DeviceMemory::get() const
  {
    return _impl->get();
  }


  class Module::Impl : boost::noncopyable
  {
  public:
    Impl(const string &file_name)
    {
      checkCuCall(cuModuleLoad(&_module, file_name.c_str()));
    }

    Impl(const void *data)
    {
      checkCuCall(cuModuleLoadData(&_module, data));
    }

    Impl(const void *data, vector<CUjit_option> &options,
         vector<void *> &optionValues)
    {
      checkCuCall(cuModuleLoadDataEx(&_module, data, options.size(),
                                     &options[0], &optionValues[0]));
    }

    ~Impl()
    {
      checkCuCall(cuModuleUnload(_module));
    }

  //private: // Function needs it to create a CUfunction
    CUmodule _module;
  };

  Module::Module(const string &file_name) : _impl(new Impl(file_name)) { }

  Module::Module(const void *data) : _impl(new Impl(data)) { }

  Module::Module(const void *data, vector<CUjit_option> &options,
                 vector<void *> &optionValues)
    : _impl(new Impl(data, options, optionValues)) { }


  Function::Function(Module &module, const string &name)
  {
    checkCuCall(cuModuleGetFunction(&_function, module._impl->_module, name.c_str()));
  }

  void Function::setArg(size_t index, const void *val)
  {
    if (index >= _kernelParams.size()) {
      _kernelParams.resize(index + 1);
    }
    _kernelParams[index] = const_cast<void *>(val);
  }

  int Function::getAttribute(CUfunction_attribute attribute) const
  {
    int value;
    checkCuCall(cuFuncGetAttribute(&value, attribute, _function));
    return value;
  }

  void Function::setSharedMemConfig(CUsharedconfig config) const
  {
    checkCuCall(cuFuncSetSharedMemConfig(_function, config));
  }


  class Event::Impl : boost::noncopyable
  {
  public:
    Impl(unsigned int flags = CU_EVENT_DEFAULT)
    {
      checkCuCall(cuEventCreate(&_event, flags));
    }

    ~Impl()
    {
      checkCuCall(cuEventDestroy(_event));
    }

    float elapsedTime(Event &second) const
    {
      float ms;
      checkCuCall(cuEventElapsedTime(&ms, second._impl->_event, _event));
      return ms;
    }

    void wait()
    {
      checkCuCall(cuEventSynchronize(_event));
    }

  //private: // Stream needs it to wait for and record events
    CUevent _event;
  };

  Event::Event(unsigned int flags) : _impl(new Impl(flags)) { }

  float Event::elapsedTime(Event &second) const
  {
    return _impl->elapsedTime(second);
  }

  void Event::wait()
  {
    _impl->wait();
  }


  class Stream::Impl : boost::noncopyable
  {
  public:
    Impl(unsigned int flags = 0)
    {
      checkCuCall(cuStreamCreate(&_stream, flags));
    }

    ~Impl()
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

    void launchKernel(CUfunction &function, dim3 gridDim, dim3 blockDim,
                      unsigned int sharedMemBytes, vector<void *>& kernelParams)
    {
      checkCuCall(cuLaunchKernel(function, gridDim.x, gridDim.y, gridDim.z,
                  blockDim.x, blockDim.y, blockDim.z, sharedMemBytes, _stream,
                  &kernelParams[0], NULL));
    }

    bool query() const
    {
      CUresult rv = cuStreamQuery(_stream);
      if (rv == CUDA_ERROR_NOT_READY) {
        return false;
      } else if (rv == CUDA_SUCCESS) {
        return true;
      }
      checkCuCall(rv); // throw
      return false; // not reached; silence compilation warning
    }

    void synchronize() const
    {
      checkCuCall(cuStreamSynchronize(_stream));
    }

    void waitEvent(CUevent event) const
    {
      checkCuCall(cuStreamWaitEvent(_stream, event, 0));
    }

    void recordEvent(CUevent event)
    {
      checkCuCall(cuEventRecord(event, _stream));
    }

  private:
    CUstream _stream;
  };

  Stream::Stream(unsigned int flags) : _impl(new Impl(flags)) { }

  void Stream::memcpyHtoDAsync(DeviceMemory &devMem, const HostMemory &hostMem,
                               size_t size)
  {
    _impl->memcpyHtoDAsync(devMem._impl->_ptr, hostMem.get<void *>(), size);
  }

  void Stream::memcpyDtoHAsync(HostMemory &hostMem, const DeviceMemory &devMem,
                               size_t size)
  {
    _impl->memcpyDtoHAsync(hostMem.get<void *>(), devMem._impl->_ptr, size);
  }

  void Stream::launchKernel(Function &function, dim3 gridDim, dim3 blockDim,
                            unsigned int sharedMemBytes)
  {
    _impl->launchKernel(function._function, gridDim, blockDim, sharedMemBytes,
                        function._kernelParams);
  }

  bool Stream::query() const
  {
    return _impl->query();
  }

  void Stream::synchronize() const
  {
    _impl->synchronize();
  }

  void Stream::waitEvent(const Event &event) const
  {
    _impl->waitEvent(event._impl->_event);
  }

  void Stream::recordEvent(const Event &event)
  {
    _impl->recordEvent(event._impl->_event);
  }


} // namespace gpu
} // namespace Cobalt
} // namespace LOFAR

