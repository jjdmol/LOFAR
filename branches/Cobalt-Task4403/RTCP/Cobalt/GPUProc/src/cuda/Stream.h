#pragma once

#include <cuda.h>  // for CUdeviceptr
#include <boost/shared_ptr.hpp>

struct Block;
class DeviceMemory;
class Event;
class Function;
struct Grid;
class HostMemory;

class Stream
{
public:
  Stream(unsigned flags = 0);
  void memcpyHtoDAsync(DeviceMemory& devMem, const HostMemory& hostMem);
  void memcpyDtoHAsync(HostMemory& hostMem, const DeviceMemory& devMem);
  void launchKernel(Function &function, const Grid&, const Block&,
                    unsigned sharedMemBytes, const void **parameters = 0);
  void query();
  void synchronize();
  void waitEvent(const Event &event);
  void record(const Event &event);
private:
  friend class Event;
  class Impl;
  boost::shared_ptr<Impl> _impl;
};
