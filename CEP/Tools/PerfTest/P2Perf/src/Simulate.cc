#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "BaseSim/SimulatorParseClass.h"
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
#else
//          cout << endl;
//  	cout << "  * Type 'define;' to define the simulation" << endl;
//  	cout <<	"  * Type 'run;'    to run the simulation" << endl;
//  	cout <<	"  * Type 'dump;'   to dump the simulators data" << endl;
//  	cout <<	"  * Type 'quit'    to quit" << endl;
//  	cout << endl;
#endif
	try {
	  P2Perf simulator;
	  simulator.setarg (argc, argv);
	  
	  Debug::initLevels (argc, (const char* [])argv);
	  try {
	    SimulatorParse::parse (simulator);
	  } catch (SimulatorParseError x) {
	    
	    //cout << x.getMesg() << endl;
	    cout << x.what() << endl;
	    
	    
	    //      P2Perf simulator;
	    //      simulator.setarg (argc, argv);
	    //      simulator.baseDefine();
	    //      //simulator.baseRun(5000);
	    //      simulator.baseRun(3651);
	    //      Simulator.baseDump();
	    //      simulator.baseQuit();
	  }
	} catch (...) {
	  cout << "Unexpected exception in Simulate" << endl;
	}
}
