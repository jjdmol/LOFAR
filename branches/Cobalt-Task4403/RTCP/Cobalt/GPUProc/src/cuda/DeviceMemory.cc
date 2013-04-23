#include "DeviceMemory.h"
#include "Error.h"

DeviceMemory::DeviceMemory(size_t bytesize)
{
  checkCudaCall(cuMemAlloc(&_ptr, bytesize));
}

DeviceMemory::~DeviceMemory()
{
  checkCudaCall(cuMemFree(_ptr));
}

CUdeviceptr DeviceMemory::get() const
{
  return _ptr;
}
