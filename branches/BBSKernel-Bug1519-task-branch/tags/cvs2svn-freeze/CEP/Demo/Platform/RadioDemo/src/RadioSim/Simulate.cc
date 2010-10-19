#include "LOFARSim.h"
#include "SimulatorParse.h"
#include <iostream>


int main (int argc, char** argv)
{
  try {
    LOFARSim simulator;
    simulator.setarg (argc, argv);
#if 0
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
	  cout << x.what() << endl;
	}
	cout << endl;
	cout << "It was a pleasure working with you!" << endl << endl;

#else
	cout << "Welcome to LOFARSim" <<endl;
	cout << "Running in batch mode " << endl;
	cout << endl;
	cout << "Call Define" << endl;
	simulator.baseDefine();
	cout << endl;
	cout << "Call Run" << endl;
	simulator.baseRun();
	cout << endl;
	cout << "Call Dump " << endl;
	simulator.baseDump();
	cout << endl;
	cout << "Good Bye!" << endl;
	simulator.baseQuit();
#endif
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
