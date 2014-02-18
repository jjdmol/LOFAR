// cuda-test.cu
// nvcc -o cuda-test cuda-test.cu
#include <cstdlib>
#include <cstring>
#include <iostream>

using std::exit;
using std::memset;
using std::cout;
using std::cerr;
using std::endl;

__global__ void kfunc(float* data) {
  float v = data[0];
  //float v = data[1024*1024*1024]; // out of bounds
  data[0] = v + 1.0f;
}

int main() {
  cudaError_t err;
  float *dptr;
  size_t len = 1024*1024;
  err = cudaMalloc((void **)&dptr, len * sizeof(float));
  if (err != cudaSuccess) { cerr << "cudaMalloc failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }

  float *hptr = new float[32*len];
  memset(hptr, 0, len * sizeof(float));
  err = cudaMemcpy(dptr, hptr, len * sizeof(float), cudaMemcpyHostToDevice);
  if (err != cudaSuccess) { cerr << "cudaMemcpy (H2D) failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }

  // launch bad kernel
  int block_dim = 1;
  int grid_dim = 1;
  kfunc<<<grid_dim, block_dim>>>(dptr);
  err = cudaGetLastError();
  if (err != cudaSuccess) { cerr << "kernel launch failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }
  err = cudaDeviceSynchronize();
  if (err != cudaSuccess) { cerr << "cudaDeviceSynchronize failed (launch) (expected): " << cudaGetErrorString(err) << " (" << err << ")" << endl; }
  if (err != cudaSuccess) {
    err = cudaGetLastError();
    if (err != cudaSuccess) { cerr << "resetting last error, which was (expected): " << cudaGetErrorString(err) << " (" << err << ")" << endl; }
    err = cudaGetLastError();
    if (err != cudaSuccess) { cerr << "reset failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; }
  } 

  err = cudaMemcpy(hptr, dptr, len * sizeof(float), cudaMemcpyDeviceToHost);
  if (err != cudaSuccess) { cerr << "cudaMemcpy (D2H) failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; }

  for (unsigned i = 0; i < 16; i++) {
    cout << hptr[i] << " ";
  }
  cout << endl;

  delete[] hptr;
  err = cudaFree(dptr);
  if (err != cudaSuccess) { cerr << "cudaFree failed: " << cudaGetErrorString(err) << " (" << err << ")" << endl; exit(1); }

  return 0;
}

