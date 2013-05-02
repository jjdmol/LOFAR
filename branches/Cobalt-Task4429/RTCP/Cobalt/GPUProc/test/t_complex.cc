#include <iostream>

#if 1
#include <GPUProc/complex.h>
using namespace LOFAR::Cobalt;
#else
#include <complex>
namespace gpu = std;
#endif

int main()
{
  gpu::complex<float> cf(3.14f, 2.72f);
  cf *= 3;
  gpu::complex<double> cd(1.618, 0.577);
  cd *= 3;
  gpu::complex<long double> cl(1.41421356237L, 1.73205080757L);
  cl *= 3;
  std::cout << cf << std::endl;
  std::cout << cd << std::endl;
  std::cout << cl << std::endl;
  cd *= gpu::complex<double>(cf);
  cf *= gpu::complex<float>(cd);
  cl *= cd;
  cf *= cl;
  std::cout << cf << std::endl;
  std::cout << cd << std::endl;
  std::cout << cl << std::endl;
  return 0;
}
