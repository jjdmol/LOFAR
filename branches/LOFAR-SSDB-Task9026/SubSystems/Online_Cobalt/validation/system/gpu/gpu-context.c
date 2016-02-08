#include <cuda.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

/*
 * This tool loops over all CUDA devices, and tries to create and destroy
 * a context on each of them.
 *
 * If this test fails on an assertion, there is something wrong with the
 * CUDA installation, the driver, or the GPUs.
 *
 * If this test fails on an alarm, the GPUs are most likely in a broken state,
 * and need to be resetted.
 */

int main()
{
  // If the GPU is in an error state, the code below will freeze indefinitely.
  // We'll activate an alarm to catch that error.

  // Note that cuInit() can take ~6 seconds on Cobalt if persistence mode is not
  // enabled, so make the timeout larger than that.
  alarm(10);

  /* Initialise the GPU and create a context */
  assert(cuInit(0) == CUDA_SUCCESS);

  int nrDevices;

  assert(cuDeviceGetCount(&nrDevices) == CUDA_SUCCESS);

  for (int d = 0; d < nrDevices; ++d) {
    CUdevice device;
    CUcontext context;

    assert(cuDeviceGet(&device, d) == CUDA_SUCCESS);
    assert(cuCtxCreate(&context, CU_CTX_SCHED_YIELD, device) == CUDA_SUCCESS);

    /* Properly clean up our resources */
    assert(cuCtxDestroy(context) == CUDA_SUCCESS);
  }

  return 0;
}
