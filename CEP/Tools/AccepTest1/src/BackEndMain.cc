//#  BackEndMain.cc: Main program for the BackEnd of the correlator
//#
//#  Copyright (C) 2002-2004
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



// TransportHolders
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <AH_BackEnd.h>
#include <TestRange.h>

#define LOCALHOST_IP "127.0.0.1"

using namespace LOFAR;

int main (int argc, const char** argv) {

  // INIT_LOGGER("CorrelatorLogger.prop");

  for (int samples = min_samples; samples <= max_samples; samples++) {
    for (int elements = min_elements; elements <= max_elements; elements++) {
      
      try {
		
	AH_BackEnd simulator(port, elements, samples,channels, runs, targets);
	
	simulator.setarg(argc, argv);
	simulator.baseDefine();
	simulator.basePrerun();
	simulator.baseRun(runs);
 	simulator.baseDump();
	simulator.baseQuit();

      } catch (LOFAR::Exception ex) {
	cout << "Caught a known exception" << endl;
	cout << ex.what() << endl;

      } catch (...) {
	cout << "Unexpected exception" << endl;
      }

    }
  }

  return 0;

}
