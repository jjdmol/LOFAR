#include "P2Perf.h"
#include "SimulatorParseClass.h"
#include <iostream>
#include "Debug.h"


#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

int main (int argc, char** argv)
{
  // Set trace level.
  Debug::initLevels (argc, (const char* [])argv);

  try {
    P2Perf simulator;
    simulator.setarg (argc, argv);

//#ifndef HAVE_MPI
#ifdef NOTDEFINED
        cout << endl;
	cout << "  * Type 'define;' to define the simulation" << endl;
	cout <<	"  * Type 'run;'    to run the simulation" << endl;
	cout <<	"  * Type 'dump;'   to dump the simulators data" << endl;
	cout <<	"  * Type 'quit'    to quit" << endl;
	cout << endl;
	try {
	  SimulatorParse::parse (simulator);
	} catch (SimulatorParseError x) {

	  //cout << x.getMesg() << endl;
	  //cout << x.what() << endl;

	}
	cout << endl;
	cout << "It was a pleasure working with you!" << endl << endl;

#else
	simulator.baseDefine();
	simulator.baseRun(3651);
	simulator.baseDump();
	simulator.baseQuit();
#endif
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
