#include "HostMemory.h"
#include "Error.h"
#include <cuda.h>

HostMemory::HostMemory(size_t bytesize, unsigned flags)
{
  checkCudaCall(cuMemHostAlloc(&_ptr, bytesize, flags));
}

HostMemory::~HostMemory()
{
  checkCudaCall(cuMemFreeHost(_ptr));
}
