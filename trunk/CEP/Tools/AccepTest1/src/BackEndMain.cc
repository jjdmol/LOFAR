//#  BackEndMain.cc: Main program for the BackEnd of the correlator
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$



// TransportHolders
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <AH_BackEnd.h>
#include <TestRange.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

#define LOCALHOST_IP "127.0.0.1"

using namespace LOFAR;

int main (int argc, const char** argv) {

  // INIT_LOGGER("CorrelatorLogger.prop");
#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);
#endif

  for (int samples = min_samples; samples <= max_samples; samples=samples+stp_samples) {
    for (int elements = min_elements; elements <= max_elements; elements=elements+stp_elements) {
      
      try {
		
	AH_BackEnd simulator(port, elements, samples, channels, polarisations, runs, targets);
	
	simulator.setarg(argc, argv);
	simulator.baseDefine();
	simulator.basePrerun();
	simulator.baseRun(runs);
 	simulator.baseDump();
	simulator.baseQuit();

// 	sleep(1);

      } catch (LOFAR::Exception ex) {
	cout << "Caught a known exception" << endl;
	cout << ex.what() << endl;

      } catch (...) {
	cout << "Unexpected exception" << endl;
      }

    }
  }

#ifdef HAVE_MPI
  TH_MPI::finalize();
#endif

  return 0;

}
