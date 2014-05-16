#include <lofar_config.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <CoInterface/Parset.h>

using namespace LOFAR;
using namespace LOFAR::Cobalt;
using namespace std;


main(int argc, char **argv)
{
  string testname("tMPISendReceive");
  std::cout << "testname" << endl;

  string parsetFile = "./tMPISendReceive.in_parset";

  // ****************************************************
  // Set up the mpi environment
  // Rank in MPI set of hosts, or 0 if no MPI is used
  int rank = 0;

  // Number of MPI hosts, or 1 if no MPI is used
  int nrHosts = 1;


  const char *rankstr, *sizestr;
  // klijn@cbt009:~/build/5607/gnu_debug/RTCP/Cobalt/GPUProc/test$ mpirun.sh -np 4 ./tMPISendReceive

  // OpenMPI rank
  if ((rankstr = getenv("OMPI_COMM_WORLD_RANK")) != NULL)
    rank = boost::lexical_cast<int>(rankstr);

  // OpenMPI size
  if ((sizestr = getenv("OMPI_COMM_WORLD_SIZE")) != NULL)
    nrHosts = boost::lexical_cast<int>(sizestr);

  // MVAPICH2 rank
  if ((rankstr = getenv("MV2_COMM_WORLD_RANK")) != NULL)
    rank = boost::lexical_cast<int>(rankstr);

  // MVAPICH2 size
  if ((sizestr = getenv("MV2_COMM_WORLD_SIZE")) != NULL)
    nrHosts = boost::lexical_cast<int>(sizestr);
  // ****************************************************

  cout <<  "MPI rank " << rank << " out of " << nrHosts << " hosts" << endl;

  Parset ps(parsetFile);


  return 0;
}
