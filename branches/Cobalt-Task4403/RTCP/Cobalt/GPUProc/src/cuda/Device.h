#pragma once

#include <string>
#include <cuda.h>  // for CUdevice_attribute

class Device
{
public:
  Device(int ordinal);
  std::string getName() const;
  template <CUdevice_attribute attribute> int getAttribute() const;
  CUdevice get() const;
private:
  CUdevice _device;
};


