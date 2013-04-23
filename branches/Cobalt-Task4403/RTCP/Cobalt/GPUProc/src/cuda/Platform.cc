#include "Platform.h"
#include "Error.h"
#include <cuda.h>

//# Initialization
Platform::Platform(unsigned flags)
{
  checkCudaCall(cuInit(flags));
}

//# Device Management
size_t Platform::size()
{
  int nrDevices;
  checkCudaCall(cuDeviceGetCount(&nrDevices));
  return nrDevices;
};


