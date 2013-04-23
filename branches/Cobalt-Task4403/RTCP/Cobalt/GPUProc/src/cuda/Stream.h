#pragma once

#include <cuda.h>  // for CUdeviceptr
#include <boost/shared_ptr.hpp>

class Event;
class Function;

class Stream
{
public:
  Stream(unsigned flags = 0);
  void memcpyHtoDAsync(CUdeviceptr devPtr, const void *hostPtr, size_t size);
  void memcpyDtoHAsync(void *hostPtr, CUdeviceptr devPtr, size_t size);
  void launchKernel(Function &function, 
                    unsigned gridX, unsigned gridY, unsigned gridZ, 
                    unsigned blockX, unsigned blockY, unsigned blockZ, 
                    unsigned sharedMemBytes, const void **parameters);
  void query();
  void synchronize();
  void waitEvent(Event &event);
  void record(Event &event);
private:
  friend class Event;
  class Impl;
  boost::shared_ptr<Impl> _impl;
};
