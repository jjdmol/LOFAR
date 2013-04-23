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

#include <lofar_config.h>

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


  class Device::Impl : boost::noncopyable
  {
  public:
    Impl(int ordinal)
    {
      checkCuCall(cuDeviceGet(&_device, ordinal));
    }

    string getName() const
    {
      // NV ref is not crystal clear on returned str len. Better be safe.
      const size_t max_name_len = 255;
      char name[max_name_len + 1];
      checkCuCall(cuDeviceGetName(name, max_name_len, _device));
      return string(name);
    }

    template <CUdevice_attribute attribute>
    int getAttribute() const
    {
      int value;
      checkCuCall(cuDeviceGetAttribute(&value, attribute, _device));
      return value;
    }

  //private: // Contest::Impl needs it to create a CUcontext
    CUdevice _device;
  };

  Device::Device(int ordinal) : _impl(new Impl(ordinal)) { }

  string Device::getName()
  {
    return _impl->getName();
  }

  template <CUdevice_attribute attribute>
  int Device::getAttribute() const
  {
    return _impl->getAttribute<attribute>();
  }


  class Context::Impl : boost::noncopyable
  {
  public:
    Impl(Device device, unsigned int flags)
    {
      checkCuCall(cuCtxCreate(&_context, flags, device._impl->_device));
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
    : _impl(new Impl(device, flags)) { }

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
         vector<void*> &optionValues)
    {
      checkCuCall(cuModuleLoadDataEx(&_module, data, options.size(),
                                     &options[0], &optionValues[0]));
    }

    ~Impl()
    {
      checkCuCall(cuModuleUnload(_module));
    }

  private:
    CUmodule _module;
  };

  Module::Module(const string &file_name) : _impl(new Impl(file_name)) { }

  Module::Module(const void *data) : _impl(new Impl(data)) { }

  Module::Module(const void *data, vector<CUjit_option> &options,
                 vector<void*> &optionValues)
    : _impl(new Impl(data, options, optionValues)) { }


} // namespace gpu
} // namespace Cobalt
} // namespace LOFAR

