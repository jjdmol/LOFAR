//# Simulate.cc
//#
//#  Copyright (C) 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$
//#

#include <StationSim/StationSim.h>
#include <BaseSim/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>


int main (int argc, char** argv)
{
  try {
    StationSim simulator;
    simulator.setarg (argc, argv);
#ifndef HAVE_MPI
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
	cout << "Welcome to StationSim" <<endl;
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
