#include <lofar_config.h>

#if defined HAVE_BGP_CN

#include <mpi.h>
#include <iostream>

#include <FCNP/fcnp_cn.h>


static char largeBuffer[128 * 1024 * 1024] __attribute__((aligned(16)));

int main(int argc, char **argv)
{
  std::vector<unsigned> psetDimensions(3);

  psetDimensions[0] = 4;
  psetDimensions[1] = 2;
  psetDimensions[2] = 2;

  int rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  FCNP_CN::init(psetDimensions);

  if (rank == 0) {
    for (size_t size = 16; size <= 128 * 1024 * 1024; size <<= 1)
       for (unsigned i = 0; i < 16; i ++)
	FCNP_CN::IONtoCN_ZeroCopy(largeBuffer, size);

    for (size_t size = 16; size <= 128 * 1024 * 1024; size <<= 1)
       for (unsigned i = 0; i < 16; i ++)
	FCNP_CN::CNtoION_ZeroCopy(largeBuffer, size);
  }

  MPI_Finalize();

  return 0;
}

#else

int main()
{
  return 0;
}

#endif // defined HAVE_BGP_CN
