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
  MPI_Init(&argc,(char***)&argv);


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
    simulator.baseRun(10); 
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
#endif
  MPI_Finalize();

}
