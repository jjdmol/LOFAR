#include "DeviceMemory.h"
#include "Error.h"

//# Memory Management
DeviceMemory::DeviceMemory(size_t bytesize) :
  _size(bytesize)
{
  checkCudaCall(cuMemAlloc(&_ptr, bytesize));
}

//# Memory Management
DeviceMemory::~DeviceMemory()
{
  checkCudaCall(cuMemFree(_ptr));
}
