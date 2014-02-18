// cu-test.cu
// nvcc -ptx cu-test.cu && nvcc -o cu-test cu-test.cu -lcuda
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cuda.h>

extern "C" {
__global__ void kfunc(float* data) {
  float v = data[0];
  //float v = data[1024*1024*1024]; // out of bounds
  data[0] = v + 1.0f;
}
}

using std::exit;
using std::memset;
using std::cout;
using std::cerr;
using std::endl;

int main() {
  CUresult r;
  r = cuInit(0);
  if (r != CUDA_SUCCESS) { cerr << "cuInit failed: " << r << endl; exit(1); }

  CUdevice dev;
  int ordinal = 0;
  r = cuDeviceGet(&dev, ordinal);
  if (r != CUDA_SUCCESS) { cerr << "cuDeviceGet failed: " << r << endl; exit(1); }

  CUcontext ctx;
  unsigned int flags = CU_CTX_SCHED_AUTO;
  r = cuCtxCreate(&ctx, flags, dev);
  if (r != CUDA_SUCCESS) { cerr << "cuCtxCreate failed: " << r << endl; exit(1); }

  CUdeviceptr dptr;
  size_t len = 1024*1024;
  r = cuMemAlloc(&dptr, len * sizeof(float));
  if (r != CUDA_SUCCESS) { cerr << "cuMemAlloc failed:" << r << endl; }

  CUdeviceptr dptr2;
  len = 1024ULL*1024*1024*1024; // too large
  r = cuMemAlloc(&dptr2, len * sizeof(float));
  if (r != CUDA_SUCCESS) { cerr << "cuMemAlloc failed (2) (expected): " << r << endl; }

  CUdeviceptr dptr3;
  len = 1024*1024; // works again after previous erroneous alloc
  r = cuMemAlloc(&dptr3, len * sizeof(float));
  if (r != CUDA_SUCCESS) { cerr << "cuMemAlloc failed (3): " << r << endl; }

  float *hptr = new float[32*len];
  memset(hptr, 0, len * sizeof(float));

  CUstream stream;
  r = cuStreamCreate(&stream, 0);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamCreate failed: " << r << endl; exit(1); }

  r = cuMemcpyHtoDAsync(dptr, hptr, len * sizeof(float), stream);
  //r = cuMemcpyHtoDAsync(dptr, hptr, 32*len * sizeof(float), stream); // GPU buffer overflow
  if (r != CUDA_SUCCESS) { cerr << "cuMemcpyHtoDAsync failed: " << r << endl; }

  r = cuStreamSynchronize(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamSynchronize failed (HtoD): " << r << endl; }

  CUmodule kmodule;
  r = cuModuleLoad(&kmodule, "cu-test.ptx");
  if (r != CUDA_SUCCESS) { cerr << "cuModuleLoad failed: " << r << endl; exit(1); }

  CUfunction kfunc;
  r = cuModuleGetFunction(&kfunc, kmodule, "kfunc");
  if (r != CUDA_SUCCESS) { cerr << "cuModuleGetFunction failed: " << r << endl; exit(1); }

  // async launch bad kernel
  void *args = &dptr;
  r = cuLaunchKernel(kfunc, /*gridDim: */1, 1, 1, /*blockDim: */1, 1, 1,
                     /*dynShmemBytes: */0, stream, &args, NULL);
  if (r != CUDA_SUCCESS) { cerr << "cuLaunchKernel failed: " << r << endl; exit(1); }

  r = cuStreamSynchronize(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamSynchronize failed (launch) (expected): " << r << endl; }

  r = cuMemcpyDtoHAsync(hptr, dptr, len * sizeof(float), stream);
  if (r != CUDA_SUCCESS) { cerr << "cuMemcpyDtoHAsync failed: " << r << endl; }

  r = cuStreamSynchronize(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamSynchronize failed (DtoH): " << r << endl; }

  for (unsigned i = 0; i < 16; i++) {
    cout << hptr[i] << " ";
  }
  cout << endl;


  delete[] hptr;
  r = cuMemFree(dptr);
  if (r != CUDA_SUCCESS) { cerr << "cuMemFree failed: " << r << endl; exit(1); }
  // delete stream, context, ...

  return 0;
}

