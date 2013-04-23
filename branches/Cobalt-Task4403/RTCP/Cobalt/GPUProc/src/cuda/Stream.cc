#include "Block.h"
#include "DeviceMemory.h"
#include "Error.h"
#include "Event.h"
#include "Function.h"
#include "Grid.h"
#include "HostMemory.h"
#include "Stream.h"
#include <boost/noncopyable.hpp>
#include <cuda.h>

class Stream::Impl : boost::noncopyable
{
public:
  //# Stream Management
  Impl(unsigned flags)
  {
    checkCudaCall(cuStreamCreate(&_stream, flags));
  }

  //# Stream Management
  ~Impl()
  {
    checkCudaCall(cuStreamDestroy(_stream));
  }

  //# Memory Management
  void memcpyHtoDAsync(CUdeviceptr devPtr, const void *hostPtr, size_t size)
  {
    checkCudaCall(cuMemcpyHtoDAsync(devPtr, hostPtr, size, _stream));
  }

  //# Memory Management
  void memcpyDtoHAsync(void *hostPtr, CUdeviceptr devPtr, size_t size)
  {
    checkCudaCall(cuMemcpyDtoHAsync(hostPtr, devPtr, size, _stream));
  }

  //# Execution Control
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

  //# Stream Management
  void query()
  {
    checkCudaCall(cuStreamQuery(_stream));
  }

  //# Stream Management
  void synchronize()
  {
    checkCudaCall(cuStreamSynchronize(_stream));
  }

  //# Stream Management
  void waitEvent(CUevent event)
  {
    checkCudaCall(cuStreamWaitEvent(_stream, event, 0));
  }

  //# Event Management
  void record(CUevent event)
  {
    checkCudaCall(cuEventRecord(event, _stream));
  }

private:
  CUstream _stream;
};


Stream::Stream(unsigned flags) :
  _impl(new Impl(flags))
{
}

void Stream::memcpyHtoDAsync(DeviceMemory& devMem, 
                             const HostMemory& hostMem)
{
  _impl->memcpyHtoDAsync(devMem._ptr, hostMem._ptr, hostMem._size);
}

void Stream::memcpyDtoHAsync(HostMemory& hostMem, 
                             const DeviceMemory& devMem)
{
  _impl->memcpyDtoHAsync(hostMem._ptr, devMem._ptr, devMem._size);
}

void Stream::launchKernel(Function &function, 
                          const Grid& grid, const Block& block,
                          unsigned sharedMemBytes, const void **parameters)
{
  _impl->launchKernel(function._function, 
                      grid.x, grid.y, grid.z, 
                      block.x, block.y, block.z, 
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

void Stream::waitEvent(const Event &event)
{
  _impl->waitEvent(event());
}

void Stream::record(const Event &event)
{
  _impl->record(event());
}
