#include <OnLineProto/OnLineProto.h>
#include <Common/lofar_iostream.h>
#include <Common/Debug.h>
#include <Common/LofarLogger.h>


using namespace LOFAR;

#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

int main (int argc, const char** argv)
{
#ifdef HAVE_MPI
  MPI_Init(&argc,(char***)&argv);
#endif
  // Set trace level.
  Debug::initLevels (argc, argv);

  // Initialize the LOFAR logger
  INIT_LOGGER("OnLineProto.log_prop");

  try {
    OnLineProto simulator;
    simulator.setarg (argc, argv);
 	simulator.baseDefine();
  	simulator.baseRun(10); 
  	simulator.baseDump();
  	simulator.baseQuit();
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
