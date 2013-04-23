#include "Device.h"
#include "Error.h"
#include <cuda.h>

Device::Device(int ordinal)
{
  checkCudaCall(cuDeviceGet(&_device, ordinal));
}

std::string Device::getName() const
{
  char name[256];
  checkCudaCall(cuDeviceGetName(name, sizeof name, _device));
  return std::string(name);
}

template <CUdevice_attribute attribute> int Device::getAttribute() const
{
  int value;
  checkCudaCall(cuDeviceGetAttribute(&value, attribute, _device));
  return value;
}

CUdevice Device::get() const
{
  return _device;
}
