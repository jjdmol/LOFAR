//#  FrontEndMain.cc: Main program for the frontend of the correlator
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#  $Id$


// TransportHolders
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <AH_FrontEnd.h>
#include <TestRange.h>

using namespace LOFAR;

int main (int argc, const char** argv) {

  // INIT_LOGGER("CorrelatorLogger.prop");

  for (int samples = min_samples; samples <= max_samples; samples++) {
    for (int elements = min_elements; elements <= max_elements; elements++) {
      
      try {		
	AH_FrontEnd frontend(port, elements, samples,channels, polarisations, runs, targets);
	
	frontend.setarg(argc, argv);
	frontend.baseDefine();
	frontend.basePrerun();
	frontend.baseRun(runs);
 	frontend.baseDump();
	frontend.baseQuit();

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
