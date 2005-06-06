//  TestAsynchronous.cc:
//
//  Copyright (C) 2000-2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
/////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <AsyncTest/AsyncTest.h>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

using namespace LOFAR;

int main (int argc, const char** argv)
{
  // Set trace level.

#ifdef HAVE_MPI
  MPI_Init(&argc, (char ***)&argv);
#else
//          cout << endl;
//  	cout << "  * Type 'define;' to define the simulation" << endl;
//  	cout <<	"  * Type 'run;'    to run the simulation" << endl;
//  	cout <<	"  * Type 'dump;'   to dump the simulators data" << endl;
//  	cout <<	"  * Type 'quit'    to quit" << endl;
//  	cout << endl;
#endif
	try {
	  LOFAR::AsyncTest simulator;
	  simulator.setarg (argc, argv);
	  
	  INIT_LOGGER("TestAsynchronous.log_prop");


	  try {
	    //	    LOFAR::SimulatorParse::parse (simulator);

	    simulator.baseDefine();
	    simulator.baseRun(500);
	    simulator.baseQuit();

	  } catch (LOFAR::SimulatorParseError x) {
	    
	    //cout << x.getMesg() << endl;
	    std::cout << x.what() << std::endl;
	    
	    
	    //      AsyncTest simulator;
	    //      simulator.setarg (argc, argv);
	    //      simulator.baseDefine();
	    //      //simulator.baseRun(5000);
	    //      simulator.baseRun(3651);
	    //      Simulator.baseDump();
	    //      simulator.baseQuit();
	  }
	} catch (...) {
	  std::cout << "Unexpected exception in TestAsynchronous" << std::endl;
	}
}
