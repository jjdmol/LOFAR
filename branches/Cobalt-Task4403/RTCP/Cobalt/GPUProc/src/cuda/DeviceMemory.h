#pragma once

#include <cstddef>
#include <cuda.h>

class DeviceMemory
{
public:
  DeviceMemory(size_t bytesize);
  ~DeviceMemory();
  CUdeviceptr get() const;
private:
  CUdeviceptr _ptr;
};

