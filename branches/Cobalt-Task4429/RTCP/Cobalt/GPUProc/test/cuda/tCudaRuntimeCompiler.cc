#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>

int main()
{
  char *kernelPath = "/home/wklijn/build/4429/gnu_debug/installed/share/gpu/kernels/TestKernel.cu";

  // Run a kernel with two different defines, should result in two results

  return 0;
}
