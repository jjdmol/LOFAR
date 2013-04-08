#include <lofar_config.h>

#include <CUDASupport/CUDAException.h>
#include <iostream>

using namespace LOFAR::Cobalt;

int main()
{
  try {
    int count;
    cudaDeviceProp prop;
    CUDA_CALL(cudaGetDeviceCount(&count));
    std::cout << "Found " << count << " CUDA capable device(s)" << std::endl;
    CUDA_CALL(cudaGetDeviceProperties(&prop, 2));
  } catch (CUDAException& e) {
    std::cerr << e << std::endl;
  }
}
