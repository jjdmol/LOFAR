#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/SimulatorParseClass.h"
#include "BaseSim/ShMem/shmem_alloc.h"
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>
#include "P2Perf/P2Perf.h"
#ifdef HAVE_MPI
#include <mpi.h>
#endif

#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

int main (int argc, char** argv)
{
  // Set trace level.

#ifdef HAVE_MPI
  MPI_Init(&argc, &argv);
#endif

  Debug::initLevels (argc, (const char* [])argv);

  try {
    P2Perf simulator;
    simulator.setarg (argc, argv);


    shmem_debug();
    simulator.baseDefine();
    //simulator.baseRun(5000);
    simulator.baseRun(3651);
    simulator.baseDump();
    simulator.baseQuit();
    
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
