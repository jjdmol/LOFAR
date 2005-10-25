#include "OnLineProto/OnLineProto.h"
#include "CEPFrame/SimulatorParseClass.h"
#include "Common/lofar_iostream.h"
#include "Common/Debug.h"
#include "Common/LofarLogger.h"


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
#ifndef HAVE_MPICH
//          cout << endl;
//  	cout << "  * Type 'define;' to define the simulation" << endl;
//  	cout <<	"  * Type 'run;'    to run the simulation" << endl;
//  	cout <<	"  * Type 'dump;'   to dump the simulators data" << endl;
//  	cout <<	"  * Type 'quit'    to quit" << endl;
//  	cout << endl;
	try {
	  LOFAR::SimulatorParse::parse (simulator);
	} catch (LOFAR::SimulatorParseError x) {

	  //cout << x.getMesg() << endl;
	  //cout << x.what() << endl;

	}
#else
  	simulator.baseDefine();
  	simulator.baseRun(10); // 1 sec worth of data
  	simulator.baseDump();
  	simulator.baseQuit();
#endif

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
