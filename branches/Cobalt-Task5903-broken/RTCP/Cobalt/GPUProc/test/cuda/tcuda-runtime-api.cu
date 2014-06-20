//# tcuda-runtime-api.cu: simple CUDA runtime API test
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

// nvcc -o tcuda-runtime-api tcuda-runtime-api.cu

//#include <lofar_config.h>

#include <cstdlib>
#include <vector>
#include <iostream>

using namespace std;

__global__ void kfunc(float* data) {
  float v = data[0];
  //float v = data[1024*1024*1024]; // intentionally out of bounds
  data[0] = v + 1.0f;
}

int main() {
  int rv = 0;
  cudaError_t err;

  float *dptr;
  size_t len = 1024*1024;
  err = cudaMalloc((void **)&dptr, len * sizeof(float));
  if (err != cudaSuccess) { cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }

  vector<float> hvec(32*len); // some extra space
  err = cudaMemcpy(dptr, &hvec[0], len * sizeof(float), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) { cerr << "cudaMemcpy (H2D) failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }

  // launch bad kernel
  int block_dim = 1;
  int grid_dim = 1;
  kfunc<<<grid_dim, block_dim>>>(dptr);
  err = cudaGetLastError();
  if (err != cudaSuccess) { cerr << "kernel launch failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }
  err = cudaDeviceSynchronize();
  if (err != cudaSuccess) { cerr << "cudaDeviceSynchronize failed (launch) (expected): " << cudaGetErrorString(err) << " (" << err << ")" << endl; rv = 1; }
  if (err != cudaSuccess) {
    err = cudaGetLastError();
    if (err != cudaSuccess) { cerr << "resetting last error, which was (expected): " << cudaGetErrorString(err) << " (" << err << ")" << endl; rv = 1; }
    err = cudaGetLastError();
    if (err != cudaSuccess) { cerr << "reset failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; rv = 1; }
  } 

  err = cudaMemcpy(&hvec[0], dptr, len * sizeof(float), cudaMemcpyDeviceToHost);
  if (err != cudaSuccess) { cerr << "cudaMemcpy (D2H) failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; rv = 1; }

  // only check first 16 output vals
  if (hvec[0] != 1.0f) { cerr << "expected hvec[0] to be 1.0f, but got " << hvec[0] << endl; rv = 1; }
  for (int i = 1; i < 16; i++) {
    if (hvec[i] != 0.0f) { cerr << "expected hvec[" << i << "] to be 0.0f, but got " << hvec[i] << endl; rv = 1; }
  }

  err = cudaFree(dptr);
  if (err != cudaSuccess) { cerr << "cudaFree failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }

  if (rv == 0)
    cout << "Test passed" << endl;
  else
    cout << "Test failed" << endl;
  return rv;
}

