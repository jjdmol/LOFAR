#include <lofar_config.h>

#include <StokesAsm.h>

#include <iostream>

using namespace LOFAR;
using namespace LOFAR::RTCP;
using namespace LOFAR::TYPES;


int main()
{
#if defined HAVE_BGP
  fcomplex samples[16][2];

  samples[14][0] = makefcomplex(2, 3);
  samples[14][1] = makefcomplex(4, 5);

  {
    float I[16], Q[16], U[16], V[16];

    _StokesIQUV(I, Q, U, V, samples, 16);

    std::cout << I[14] << ' ' << Q[14] << ' ' << U[14] << ' ' << V[14] << std::endl;
  }

  {
    float Xr = real(samples[14][0]);
    float Xi = imag(samples[14][0]);
    float Yr = real(samples[14][1]);
    float Yi = imag(samples[14][1]);

    float Xr2 = Xr * Xr;
    float Xi2 = Xi * Xi;
    float Yr2 = Yr * Yr;
    float Yi2 = Yi * Yi;

    float I = Xr2 + Xi2 + Yr2 + Yi2;
    float Q = Xr2 + Xi2 - Yr2 - Yi2;
    float U = Xr * Yr + Xi * Yi;
    float V = Xi * Yr - Xr * Yi;

    std::cout << I << ' ' << Q << ' ' << U << ' ' << V << std::endl;
  }
#endif

  return 0;
}
