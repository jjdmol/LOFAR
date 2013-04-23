#include "HostMemory.h"
#include "Error.h"
#include <cuda.h>

//# Memory Management
HostMemory::HostMemory(size_t bytesize, unsigned flags) :
  _size(bytesize)
{
  checkCudaCall(cuMemHostAlloc(&_ptr, bytesize, flags));
}

//# Memory Management
HostMemory::~HostMemory()
{
  checkCudaCall(cuMemFreeHost(_ptr));
}
