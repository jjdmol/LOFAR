#include <GPUProc/complex.h>
#include <GPUProc/cuda/CUDAException.h>
#include <iostream>

using namespace LOFAR::Cobalt;

__global__ void kernel()
{
  /* gpu::complex<float> cf(3.14f, 2.72f); */
  /* cf *= 3.0f; */
  /* gpu::complex<double> cd(1.618, 0.577); */
  /* cd *= 3.0; */
  /* gpu::complex<long double> cl(1.41421356237L, 1.73205080757L); */
  /* cl *= 3.0L; */
  /* std::cout << cf << std::endl; */
  /* std::cout << cd << std::endl; */
  /* std::cout << cl << std::endl; */
  /* cd *= cf; */
  /* cl *= cd; */
  /* cf *= cl; */
  /* std::cout << cf << std::endl; */
  /* std::cout << cd << std::endl; */
  /* std::cout << cl << std::endl; */
}

using namespace std;

int main()
{
  std::cout << "Executing kernel" << std::endl;
  kernel<<<1,1>>>();
  CUDA_CALL(cudaGetLastError());
  gpu::complex<float> cf(3.14f, 2.72f);
  cf *= 3.0f;
  gpu::complex<double> cd(1.618, 0.577);
  cd *= 3.0;
  gpu::complex<long double> cl(1.41421356237L, 1.73205080757L);
  cl *= 3.0L;
  std::cout << cf << std::endl;
  std::cout << cd << std::endl;
  std::cout << cl << std::endl;
  cd *= cf;
  cl *= cd;
  cf *= cl;
  std::cout << cf << std::endl;
  std::cout << cd << std::endl;
  std::cout << cl << std::endl;
  std::cout << "Done" << std::endl;
  return 0;
}
