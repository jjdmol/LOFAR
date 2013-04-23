#pragma once

#include "Stream.h"
#include "Error.h"
#include "Event.h"
#include "Function.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

#include "Event.cc"

class Stream::Impl : boost::noncopyable
{
public:
  Impl(unsigned flags)
  {
    checkCudaCall(cuStreamCreate(&_stream, flags));
  }

  ~Impl()
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

  void launchKernel(CUfunction function, 
                    unsigned gridX, unsigned gridY, unsigned gridZ, 
                    unsigned blockX, unsigned blockY, unsigned blockZ, 
                    unsigned sharedMemBytes, const void **parameters)
  {
    checkCudaCall(cuLaunchKernel(function, 
                                 gridX, gridY, gridZ, 
                                 blockX, blockY, blockZ, 
                                 sharedMemBytes, _stream, 
                                 const_cast<void **>(parameters), 0));
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
    checkCudaCall(cuStreamWaitEvent(_stream, event._impl->_event, 0));
  }

  void record(Event &event)
  {
    checkCudaCall(cuEventRecord(event._impl->_event, _stream));
  }

private:
  CUstream _stream;
};


Stream::Stream(unsigned flags) :
  _impl(new Impl(flags))
{
}

void Stream::memcpyHtoDAsync(CUdeviceptr devPtr, 
                             const void *hostPtr, 
                             size_t size)
{
  _impl->memcpyHtoDAsync(devPtr, hostPtr, size);
}

void Stream::memcpyDtoHAsync(void *hostPtr, 
                             CUdeviceptr devPtr, 
                             size_t size)
{
  _impl->memcpyDtoHAsync(hostPtr, devPtr, size);
}

void Stream::launchKernel(Function &function, 
                          unsigned gridX, unsigned gridY, unsigned gridZ, 
                          unsigned blockX, unsigned blockY, unsigned blockZ, 
                          unsigned sharedMemBytes, const void **parameters)
{
  _impl->launchKernel(function._function, 
                      gridX, gridY, gridZ, 
                      blockX, blockY, blockZ, 
                      sharedMemBytes, parameters);
}

void Stream::query()
{
  _impl->query();
}

void Stream::synchronize()
{
  _impl->synchronize();
}

void Stream::waitEvent(Event &event)
{
  _impl->waitEvent(event);
}

void Stream::record(Event &event)
{
  _impl->record(event);
}
