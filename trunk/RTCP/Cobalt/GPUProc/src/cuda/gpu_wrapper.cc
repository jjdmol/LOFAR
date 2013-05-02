//# gpu_wrapper.cc: CUDA-specific wrapper classes for GPU types.
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

#include <lofar_config.h>
#include "gpu_wrapper.h"
#include <boost/noncopyable.hpp>
#include <string>
#include <algorithm>  // for std::min

// Convenience macro to call a CUDA Device API function and throw a
// CUDAException if an error occurred.
#define checkCuCall(func)                                               \
  do {                                                                  \
    CUresult result = func;                                             \
    if (result != CUDA_SUCCESS) {                                       \
      THROW (LOFAR::Cobalt::gpu::CUDAException,                         \
             # func << ": " << LOFAR::Cobalt::gpu::errorMessage(result)); \
    }                                                                   \
  } while(0)


namespace LOFAR
{
  namespace Cobalt
  {
    namespace gpu
    {

      std::string errorMessage(CUresult errcode)
      {
        switch (errcode) {
        case CUDA_SUCCESS:
          return "Success";
        case CUDA_ERROR_INVALID_VALUE:
          return "Invalid value";
        case CUDA_ERROR_OUT_OF_MEMORY:
          return "Out of memory";
        case CUDA_ERROR_NOT_INITIALIZED:
          return "Not initialized";
        case CUDA_ERROR_DEINITIALIZED:
          return "Deinitialized";
        case CUDA_ERROR_PROFILER_DISABLED:
          return "Profiler disabled";
        case CUDA_ERROR_PROFILER_NOT_INITIALIZED:
          return "Profiler not initialized";
        case CUDA_ERROR_PROFILER_ALREADY_STARTED:
          return "Profiler already started";
        case CUDA_ERROR_PROFILER_ALREADY_STOPPED:
          return "Profiler already stopped";
        case CUDA_ERROR_NO_DEVICE:
          return "No device";
        case CUDA_ERROR_INVALID_DEVICE:
          return "Invalid device";
        case CUDA_ERROR_INVALID_IMAGE:
          return "Invalid image";
        case CUDA_ERROR_INVALID_CONTEXT:
          return "Invalid context";
        case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:
          return "Context already current";
        case CUDA_ERROR_MAP_FAILED:
          return "Map failed";
        case CUDA_ERROR_UNMAP_FAILED:
          return "Unmap failed";
        case CUDA_ERROR_ARRAY_IS_MAPPED:
          return "Array is mapped";
        case CUDA_ERROR_ALREADY_MAPPED:
          return "Already mapped";
        case CUDA_ERROR_NO_BINARY_FOR_GPU:
          return "No binary for GPU";
        case CUDA_ERROR_ALREADY_ACQUIRED:
          return "Already acquired";
        case CUDA_ERROR_NOT_MAPPED:
          return "Not mapped";
        case CUDA_ERROR_NOT_MAPPED_AS_ARRAY:
          return "Not mapped as array";
        case CUDA_ERROR_NOT_MAPPED_AS_POINTER:
          return "Not mapped as pointer";
        case CUDA_ERROR_ECC_UNCORRECTABLE:
          return "ECC uncorrectable";
        case CUDA_ERROR_UNSUPPORTED_LIMIT:
          return "Unsupported limit";
        case CUDA_ERROR_CONTEXT_ALREADY_IN_USE:
          return "Context already in use";
        case CUDA_ERROR_INVALID_SOURCE:
          return "Invalid source";
        case CUDA_ERROR_FILE_NOT_FOUND:
          return "File not found";
        case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND:
          return "Shared object symbol not found";
        case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED:
          return "Shared object init failed";
        case CUDA_ERROR_OPERATING_SYSTEM:
          return "Operating system";
        case CUDA_ERROR_INVALID_HANDLE:
          return "Invalid handle";
        case CUDA_ERROR_NOT_FOUND:
          return "Not found";
        case CUDA_ERROR_NOT_READY:
          return "Not ready";
        case CUDA_ERROR_LAUNCH_FAILED:
          return "Launch failed";
        case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:
          return "Launch out of resources";
        case CUDA_ERROR_LAUNCH_TIMEOUT:
          return "Launch timeout";
        case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:
          return "Launch incompatible texturing";
        case CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED:
          return "Peer access already enabled";
        case CUDA_ERROR_PEER_ACCESS_NOT_ENABLED:
          return "Peer access not enabled";
        case CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE:
          return "Primary context active";
        case CUDA_ERROR_CONTEXT_IS_DESTROYED:
          return "Context is destroyed";
        case CUDA_ERROR_UNKNOWN:
          return "Unknown";
#if CUDA_VERSION >= 4010
        case CUDA_ERROR_ASSERT:
          return "Assert";
        case CUDA_ERROR_TOO_MANY_PEERS:
          return "Too many peers";
        case CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED:
          return "Host memory already registered";
        case CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED:
          return "Host memory not registered";
#endif
#if CUDA_VERSION >= 5000
        case CUDA_ERROR_PEER_ACCESS_UNSUPPORTED:
          return "Peer access unsupported";
        case CUDA_ERROR_NOT_PERMITTED:
          return "Not permitted";
        case CUDA_ERROR_NOT_SUPPORTED:
          return "Not supported";
#endif
        default:
          std::stringstream str;
          str << "Unknown error (" << errcode << ")";
          return str.str();
        }
      }


      Block::Block(unsigned int x_, unsigned int y_, unsigned int z_) :
        x(x_), y(y_), z(z_)
      {
      }


      Grid::Grid(unsigned int x_, unsigned int y_, unsigned int z_) :
        x(x_), y(y_), z(z_)
      {
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

      std::string Device::getName() const
      {
        // NV ref is not crystal clear on returned str len. Better be safe.
        const size_t max_name_len = 255;
        char name[max_name_len + 1];
        checkCuCall(cuDeviceGetName(name, max_name_len, _device));
        return std::string(name);
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
#if CUDA_VERSION >= 4020
          checkCuCall(cuCtxSetSharedMemConfig(config));
#else
          (void)config;
#endif
        }

      private:
        CUcontext _context;
      };

      Context::Context(Device device, unsigned int flags) :
        _impl(new Impl(device._device, flags))
      {
      }

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
        Impl(size_t size, unsigned int flags) : _size(size)
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

        size_t size() const
        {
          return _size;
        }

      private:
        void *_ptr;
        size_t _size;
      };

      HostMemory::HostMemory(size_t size, unsigned int flags) :
        _impl(new Impl(size, flags))
      {
      }

      size_t HostMemory::size() const
      {
        return _impl->size();
      }

      void* HostMemory::getPtr() const
      {
        return _impl->get();
      }


      class DeviceMemory::Impl : boost::noncopyable
      {
      public:
        Impl(size_t size) : _size(size)
        {
          checkCuCall(cuMemAlloc(&_ptr, size));
        }

        ~Impl()
        {
          checkCuCall(cuMemFree(_ptr));
        }

        size_t size() const
        {
          return _size;
        }

        //private: // Stream needs it to do transfers
        CUdeviceptr _ptr;
      private:
        size_t _size;
      };

      DeviceMemory::DeviceMemory(size_t size) :
        _impl(new Impl(size))
      {
      }

      size_t DeviceMemory::size() const
      {
        return _impl->size();
      }


      class Module::Impl : boost::noncopyable
      {
      public:
        Impl(const char* fname)
        {
          checkCuCall(cuModuleLoad(&_module, fname));
        }

        Impl(const void *data)
        {
          checkCuCall(cuModuleLoadData(&_module, data));
        }

        Impl(const void *image, unsigned int numOptions,
             CUjit_option *options, void **optionValues)
        {
          checkCuCall(cuModuleLoadDataEx(&_module, image, numOptions,
                                         options, optionValues));
        }

        ~Impl()
        {
          checkCuCall(cuModuleUnload(_module));
        }

        //private: // Function needs it to create a CUfunction
        CUmodule _module;
      };

      Module::Module(const std::string &fname) :
        _impl(new Impl(fname.c_str()))
      {
      }

      Module::Module(const void *image) :
        _impl(new Impl(image))
      {
      }

      Module::Module(const void *image,
                     std::vector<CUjit_option> &options,
                     std::vector<void*> &optionValues) :
        _impl(new Impl(image, std::min(options.size(), optionValues.size()),
                       &options[0], &optionValues[0]))
      {
      }


      Function::Function(Module &module, const std::string &name)
      {
        checkCuCall(cuModuleGetFunction(&_function, module._impl->_module,
                                        name.c_str()));
      }

      int Function::getAttribute(CUfunction_attribute attribute) const
      {
        int value;
        checkCuCall(cuFuncGetAttribute(&value, attribute, _function));
        return value;
      }

      void Function::setSharedMemConfig(CUsharedconfig config) const
      {
#if CUDA_VERSION >= 4020
        checkCuCall(cuFuncSetSharedMemConfig(_function, config));
#else
        (void)config;
#endif
      }


      class Event::Impl : boost::noncopyable
      {
      public:
        Impl(unsigned int flags)
        {
          checkCuCall(cuEventCreate(&_event, flags));
        }

        ~Impl()
        {
          checkCuCall(cuEventDestroy(_event));
        }

        float elapsedTime(CUevent other) const
        {
          float ms;
          checkCuCall(cuEventElapsedTime(&ms, other, _event));
          return ms;
        }

        void wait()
        {
          checkCuCall(cuEventSynchronize(_event));
        }

        //private: // Stream needs it to wait for and record events
        CUevent _event;
      };

      Event::Event(unsigned int flags) : _impl(new Impl(flags))
      {
      }

      float Event::elapsedTime(Event &second) const
      {
        return _impl->elapsedTime(second._impl->_event);
      }

      void Event::wait()
      {
        _impl->wait();
      }


      class Stream::Impl : boost::noncopyable
      {
      public:
        Impl(unsigned int flags)
        {
          checkCuCall(cuStreamCreate(&_stream, flags));
        }

        ~Impl()
        {
          checkCuCall(cuStreamDestroy(_stream));
        }

        void memcpyHtoDAsync(CUdeviceptr devPtr, const void *hostPtr, 
                             size_t size)
        {
          checkCuCall(cuMemcpyHtoDAsync(devPtr, hostPtr, size, _stream));
        }

        void memcpyDtoHAsync(void *hostPtr, CUdeviceptr devPtr, size_t size)
        {
          checkCuCall(cuMemcpyDtoHAsync(hostPtr, devPtr, size, _stream));
        }

        void launchKernel(CUfunction function, unsigned gridX, unsigned gridY,
                          unsigned gridZ, unsigned blockX, unsigned blockY,
                          unsigned blockZ, unsigned sharedMemBytes,
                          void **parameters)
        {
          checkCuCall(cuLaunchKernel(function, gridX, gridY, gridZ, blockX,
                                     blockY, blockZ, sharedMemBytes, _stream,
                                     parameters, 0));
        }

        bool query() const
        {
          CUresult rv = cuStreamQuery(_stream);
          if (rv == CUDA_ERROR_NOT_READY) {
            return false;
          } else if (rv == CUDA_SUCCESS) {
            return true;
          }
          checkCuCall(rv); // check and throw if other error
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

      Stream::Stream(unsigned int flags) : _impl(new Impl(flags))
      {
      }

      void Stream::writeBuffer(DeviceMemory &devMem, 
                               const HostMemory &hostMem,
                               bool synchronous)
      {
        _impl->memcpyHtoDAsync(devMem._impl->_ptr, 
                               hostMem.get<void*>(), 
                               hostMem.size());
        if (synchronous) {
          synchronize();
/*
          Event ev;
          recordEvent(ev);
          // waitEvent(ev) fails here, as it syncs across streams, the host doesn't have to wait
          ev.wait();
*/
        }
      }

      void Stream::readBuffer(HostMemory &hostMem, 
                              const DeviceMemory &devMem,
                              bool synchronous)
      {
        _impl->memcpyDtoHAsync(hostMem.get<void*>(), 
                               devMem._impl->_ptr, 
                               devMem.size());
        if (synchronous) {
          synchronize();
/*
          Event ev;
          recordEvent(ev);
          // waitEvent(ev) fails here, as it syncs across streams, the host doesn't have to wait
          ev.wait();
*/
        }
      }

      void Stream::launchKernel(const Function &function,
                                const Grid &grid, const Block &block,
                                unsigned sharedMemBytes, 
                                const void **parameters)
      {
        _impl->launchKernel(function._function,
                            grid.x, grid.y, grid.z, block.x, block.y, block.z,
                            sharedMemBytes, const_cast<void **>(parameters));
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

