#include <BlueGeneCorrelator.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>

#include <BlueGeneDefinitions.h>
#include <mpi.h>

#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

using namespace LOFAR;


int main (int argc, const char** argv)
{

#ifdef __BLRTS__

  MPI_Init(&argc,(char***)&argv);

  /* Define a complex float datatype */
  MPI_Type_contiguous(2, MPI_FLOAT, &my_complex);
  MPI_Type_commit(&my_complex);

  // Set trace level.
  Debug::initLevels (argc, argv);

  // The BlueGene 
  try {
    BlueGeneCorrelator simulator(true);
    simulator.setarg (argc, argv);
    
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(10); 
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
  MPI_Finalize();

#endif
}
