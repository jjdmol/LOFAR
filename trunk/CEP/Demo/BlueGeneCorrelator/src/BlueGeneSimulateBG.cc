#include <BlueGeneCorrelator.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>
#include <Common/LofarLogger.h>

#include <BlueGeneCorrelator/definitions.h>

#ifdef __BLRTS__
#include <mpi.h>
#endif

#ifdef __MPE_LOGGING__
#include <mpe.h>
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

  int myrank = -1;

  INIT_LOGGER("BGlogger.prop");

#ifdef __BLRTS__
  MPI_Init(&argc,(char***)&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
#endif

#ifdef __MPE_LOGGING__
  MPE_Init_log();
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

    // the slaves only do one process step
    if (myrank != 0) { 
      simulator.baseRun(1); 
    } else  {
      simulator.baseRun(RUNS);
    }

    simulator.basePostrun();
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
#endif

#ifdef __MPE_LOGGING__
  MPE_Finish_log("BGcorrelator");
#endif

#ifdef __BLRTS__
  MPI_Finalize();
#endif

}
