#include "Platform.h"
#include "Error.h"
#include <cuda.h>

Platform::Platform(unsigned flags)
{
  checkCudaCall(cuInit(flags));
}

size_t Platform::size()
{
  int nrDevices;
  checkCudaCall(cuDeviceGetCount(&nrDevices));
  return nrDevices;
};


