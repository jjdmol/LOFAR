//# tcuda-driver-api.cu: simple CUDA driver API test
//# Copyright (C) 2014  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

// nvcc -ptx tcuda-driver-api.cu && nvcc -o tcuda-driver-api tcuda-driver-api.cu -lcuda

//#include <lofar_config.h>

#include <cstdlib>
#include <vector>
#include <iostream>
#include <cuda.h>

#ifndef PTX_FILE_CSTR
#define PTX_FILE_CSTR "tcuda-driver-api.ptx"
#endif

extern "C" {
__global__ void kfunc(float* data) {
  float v = data[0];
  //float v = data[1024*1024*1024]; // intentionally out of bounds
  data[0] = v + 1.0f;
}
}

using namespace std;

int main() {
  int rv = 0;
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
  if (r != CUDA_SUCCESS) { cerr << "cuMemAlloc failed:" << r << endl; rv = 1; }

/*
  CUdeviceptr dptr2;
  len = 1024ULL*1024*1024*1024; // too large
  r = cuMemAlloc(&dptr2, len * sizeof(float));
  if (r != CUDA_SUCCESS) { cerr << "cuMemAlloc failed (2) (expected): " << r << endl; rv = 1; }
*/

  CUdeviceptr dptr3;
  len = 1024*1024; // works again after previous erroneous alloc (if enabled at all)
  r = cuMemAlloc(&dptr3, len * sizeof(float));
  if (r != CUDA_SUCCESS) { cerr << "cuMemAlloc failed (3): " << r << endl; rv = 1; }

  vector<float> hvec(32*len); // some extra space

  CUstream stream;
  r = cuStreamCreate(&stream, 0);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamCreate failed: " << r << endl; exit(1); }

  r = cuMemcpyHtoDAsync(dptr, &hvec[0], len * sizeof(float), stream);
  //r = cuMemcpyHtoDAsync(dptr, &hvec[0], 32*len * sizeof(float), stream); // GPU buffer overflow
  if (r != CUDA_SUCCESS) { cerr << "cuMemcpyHtoDAsync failed: " << r << endl; rv = 1; }

  r = cuStreamSynchronize(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamSynchronize failed (HtoD): " << r << endl; rv = 1; }

  cout << "Trying to load module file " << PTX_FILE_CSTR << endl;
  CUmodule kmodule;
  r = cuModuleLoad(&kmodule, PTX_FILE_CSTR); // should have been precompiled externally
  if (r != CUDA_SUCCESS) { cerr << "cuModuleLoad failed: " << r << endl; exit(1); }
  cout << "Module load succeeded" << endl;

  CUfunction kfunc;
  r = cuModuleGetFunction(&kfunc, kmodule, "kfunc");
  if (r != CUDA_SUCCESS) { cerr << "cuModuleGetFunction failed: " << r << endl; exit(1); }

  // async launch bad kernel
  void *args = &dptr;
  r = cuLaunchKernel(kfunc, /*gridDim: */1, 1, 1, /*blockDim: */1, 1, 1,
                     /*dynShmemBytes: */0, stream, &args, NULL);
  if (r != CUDA_SUCCESS) { cerr << "cuLaunchKernel failed: " << r << endl; exit(1); }

  r = cuStreamSynchronize(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamSynchronize failed (launch) (expected): " << r << endl; rv = 1; }

  r = cuMemcpyDtoHAsync(&hvec[0], dptr, len * sizeof(float), stream);
  if (r != CUDA_SUCCESS) { cerr << "cuMemcpyDtoHAsync failed: " << r << endl; rv = 1; }

  r = cuStreamSynchronize(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamSynchronize failed (DtoH): " << r << endl; rv = 1; }

  // only check first 16 output vals
  if (hvec[0] != 1.0f) { cerr << "expected hvec[0] to be 1.0f, but got " << hvec[0] << endl; rv = 1; }
  for (int i = 1; i < 16; i++) {
    if (hvec[i] != 0.0f) { cerr << "expected hvec[" << i << "] to be 0.0f, but got " << hvec[i] << endl; rv = 1; }
  }

  r = cuModuleUnload(kmodule);
  if (r != CUDA_SUCCESS) { cerr << "cuModuleUnload failed: " << r << endl; exit(1); }
  r = cuStreamDestroy(stream);
  if (r != CUDA_SUCCESS) { cerr << "cuStreamDestroy failed: " << r << endl; exit(1); }
  r = cuMemFree(dptr3);
  if (r != CUDA_SUCCESS) { cerr << "cuMemFree dptr3 failed: " << r << endl; exit(1); }
  r = cuMemFree(dptr);
  if (r != CUDA_SUCCESS) { cerr << "cuMemFree dptr failed: " << r << endl; exit(1); }
  r = cuCtxDestroy(ctx);
  if (r != CUDA_SUCCESS) { cerr << "cuCtxDestroy failed: " << r << endl; exit(1); }

  cout << "Test passed" << endl;
  return rv;
}

