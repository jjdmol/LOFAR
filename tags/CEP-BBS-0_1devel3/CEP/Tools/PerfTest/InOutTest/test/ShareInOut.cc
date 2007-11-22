//#  ShareInOut.cc:
//#
//#  Copyright (C) 2000-2002
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>

#include <InOutTest/InOutTest.h>

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
	  INIT_LOGGER("ShareInOut.log_prop");

	  InOutTest simulator;
	  simulator.setarg (argc, argv);

	  simulator.baseDefine();
	  simulator.baseRun(10);
	  simulator.baseQuit();	  
	}
	catch (LOFAR::Exception& e)
	{
	  cout << "Lofar exception: " << e.what() << endl;
	}
	catch (std::exception& e)
	{
	  cout << "Standard exception: " << e.what() << endl;
	}
	catch (...) {
	  cout << "Unexpected exception in ShareInOut" << endl;
	}

}
