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
#include <Common/KeyParser.h>
#include <Common/KeyValueMap.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <AH_BackEnd.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

#define LOCALHOST_IP "127.0.0.1"

using namespace LOFAR;

int main (int argc, const char** argv) {

  KeyValueMap kvm;
  try {
    kvm = KeyParser::parseFile("TestRange");
  } catch (std::exception& x) {
    cout << x.what() << endl;
  }
  
  const int min_elements = kvm.getInt("min_elements", 50);
  const int max_elements = kvm.getInt("max_elements", 100);
  const int stp_elements = kvm.getInt("stp_elements", 5);

  const int min_samples = kvm.getInt("min_samples", 100);
  const int max_samples = kvm.getInt("max_samples", 1000);
  const int stp_samples = kvm.getInt("stp_samples", 100);
  
  const int port = kvm.getInt("port", 1100);
  const int channels = kvm.getInt("channels", 1);
  const int polarisations = kvm.getInt("polarisations", 2);
  const int runs = kvm.getInt("runs", 10);
  const int targets = kvm.getInt("targets", 8);

//   const std::string frontend_ip = kvm.getString("frontend_ip");
//   const std::string backend_ip = kvm.getString("backend_ip");
  
  const std::string loggerfile = kvm.getString("loggerfile", "CorrelatorLogger.prop");

  kvm.show(cout);

  INIT_LOGGER(loggerfile);

#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);
#endif

  for (int samples = min_samples; samples <= max_samples; samples=samples+stp_samples) {
    for (int elements = min_elements; elements <= max_elements; elements=elements+stp_elements) {
      
      try {
	AH_BackEnd* simulator;

	if ((samples+elements) % 2 == 0) {
	  simulator = new AH_BackEnd(port, elements, samples, channels, polarisations, runs, targets);
	} else {
	  simulator = new AH_BackEnd(port+2*targets, elements, samples, channels, polarisations, runs, targets);
	}
	
	simulator->setarg(argc, argv);
	simulator->baseDefine();
	simulator->basePrerun();
	simulator->baseRun(runs);
 	simulator->baseDump();
	simulator->baseQuit();

	delete simulator;

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
