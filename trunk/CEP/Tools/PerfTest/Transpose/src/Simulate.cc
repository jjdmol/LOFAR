#include "Transpose/Transpose.h"
#include "BaseSim/SimulatorParseClass.h"
#include "Common/lofar_iostream.h"
#include "Common/Debug.h"


#ifdef HAVE_CORBA
int atexit(void (*function)(void))
{
  return 1;
}
#endif

int main (int argc, char** argv)
{
#ifdef HAVE_MPI
  MPI_Init(&argc,&argv);
#endif
  // Set trace level.
  Debug::initLevels (argc, (const char* [])argv);

  try {
    Transpose simulator;
    simulator.setarg (argc, argv);
#ifndef HAVE_MPI
        cout << endl;
	cout << "  * Type 'define;' to define the simulation" << endl;
	cout <<	"  * Type 'run;'    to run the simulation" << endl;
	cout <<	"  * Type 'dump;'   to dump the simulators data" << endl;
	cout <<	"  * Type 'quit'    to quit" << endl;
	cout << endl;
#endif
	try {
	  SimulatorParse::parse (simulator);
	} catch (SimulatorParseError x) {

	  //cout << x.getMesg() << endl;
	  //cout << x.what() << endl;

	}

//  	simulator.baseDefine();
//  	simulator.baseRun(1000); // 1 sec worth of data
//  	simulator.baseDump();
//  	simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
