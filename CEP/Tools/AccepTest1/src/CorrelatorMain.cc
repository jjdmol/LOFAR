//#  CorrelatorMain: Main application for the correlatornodes
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

#include <AH_Correlator.h>
#include <TestRange.h>

using namespace LOFAR;
int main (int argc, const char** argv) {

  //INIT_LOGGER("CorrelatorLogger.prop");
#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);

  if (TH_MPI::getCurrentRank() < targets) {
#else 
  if (true) {
#endif

    for (int samples = min_samples; samples <= max_samples; samples++) {
      for (int elements = min_elements; elements <= max_elements; elements++) {
	
	// init the MPI environment.
	
	try {
	  
	  AH_Correlator correlator(elements, samples, channels, frontend_ip, port, targets);
	  correlator.setarg(argc, argv);
	  
	  /* Automatic run of the correlator */
	  
	  correlator.baseDefine();
	  correlator.basePrerun();
	  
	  correlator.baseRun(runs);
	  
	  correlator.basePostrun();
	  // 	correlator.baseDump();
	  correlator.baseQuit();
	  
	} catch (LOFAR::Exception ex) {
	  // catch known exceptions
	  cout << "Caught a known exception" << endl;
	  cout << ex.what() << endl;
	  
	} catch (...) {
	  
	  cout << "Unexpected exception" << endl;
	  
	}
	
      }
    }
  }

#ifdef HAVE_MPI
  // finalize the MPI environment
  TH_MPI::finalize();
#endif

  return 0;
}
