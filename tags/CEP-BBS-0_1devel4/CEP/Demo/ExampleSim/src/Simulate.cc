//  Simulate.cc:
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
#include <Common/LofarLogger.h>
#include <ExampleSim/ExampleSim.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif

using namespace LOFAR;

int main (int argc, const char** argv)
{

#ifdef HAVE_MPI
  MPI_Init(&argc, (char ***)&argv);
#endif

  INIT_LOGGER("ExampleSim.log_prop");

  try {
    ExampleSim simulator;
    simulator.setarg (argc, argv);
	  
    try {
      LOFAR::SimulatorParse::parse (simulator);
    }
    catch (LOFAR::SimulatorParseError x) {
	    
      cout << x.what() << endl;       
    }
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
    cout << "Unexpected exception in Simulate" << endl;
  }
}
