#include <BlueGeneCorrelator.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>

#include <BlueGeneCorrelator/definitions.h>

#ifdef __BLRTS__
#include <mpi.h>
#endif
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
#endif

  // Set trace level.
  Debug::initLevels (argc, argv);

  BlueGeneCorrelator simulator;
  simulator.setarg (argc, argv);

#if 0
    try {
      LOFAR::SimulatorParse::parse(simulator);
    } catch (LOFAR::SimulatorParseError x) {
      cout << x.what() << endl;
    }
#else
  // The BlueGene 
  try {
    
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(RUNS); 
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
#endif

#ifdef __BLRTS__
  MPI_Finalize();
#endif

}
