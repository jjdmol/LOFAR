#pragma once

#include <cstddef>
#include <cuda.h>

class DeviceMemory
{
public:
  DeviceMemory(size_t bytesize);
  ~DeviceMemory();
private:
  friend class Stream;
  CUdeviceptr _ptr;
  size_t _size;
};

