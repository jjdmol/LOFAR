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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tinyCEP/SimulatorParseClass.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <Common/KeyParser.h>
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


  LOFAR::KeyValueMap kvm;
  try {
    kvm = KeyParser::parseFile("TestRange");
  } catch (std::exception x) {
    cerr << x.what() << endl;
  }

  P2Perf simulator;
  try {
    simulator.setarg (argc, argv);
    try {
#ifdef HAVE_MPI 
      // with MPI it is impossible to read from stdin
      
      // these are the possible options to give this program, either on the command line or in this kvm
      //
      // shmem = 0|1                        use TH_ShMem (only works when compiled with MPI)
      // use_sockets = 0|1                  use TH_Socket (do not compile with MPI or CORBA)
      // destside = 0|1                     use this to tell the program if it is started on the destination side or the sending side (usefull for Sockets, MPI doesn't need this)
      // sockets_sending_host = <host>      the hostname of the sending host
      // sockets_receiving_host = <host>    the hostname of the receiving host
      //                                    sockets can only be used from one host to one other host; on the receiving host, the program needs to be started with destside = 1
      // sockets_portnumber = <number>      the portnumber to use for the socket
      
      // destinations = n                   the number of destinations steps (default = 1)
      // sources = n                        the number of source steps (default = 1)
      // packets_per_meas = n               number of packets in one measurement
      // meas_per_step = n                  number of measurements at the same packet size
      // grow_strategy = exp | fixed | lin  the growth strategy: fixed (no growth), exponential or lineair
      // grow_factor = n                    the grow factor per step (for exp growth), can be smaller than 1
      // grow_increment = n                 the grow increment per step (for lineair growth), can be negative
      // initial_size = n                   the initial (or fixed) size
      
      // the number given at the run statement determines what the eventual size is:
      // eventual size = initial_size * grow_factor ^ ( runs / (packets_per_meas * meas_per_step))            or
      // eventual size = initial_size + grow_increment * runs / (packets_per_meas * meas_per_step)
     
      simulator.baseDefine(kvm);
      simulator.baseRun(4050);
      //simulator.baseDump();
      simulator.baseQuit();
#else
      LOFAR::SimulatorParse::parse (simulator);
#endif
    } catch (LOFAR::SimulatorParseError x) {
      
      cout << x.what() << endl;
      
    }
  } catch (std::exception e) {
    
    cout << "Unexpected exception in Simulate: " << e.what() << endl;
    
  }
  
}
