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

  int be_rank = -1;

  if (argc != 2) {
    cout << "Usage " << argv[0] << " <BackEnd rank>" << endl;
    cout << "For each targetgroup the user needs to start a BackEnd" << endl;
    cout << "with the corresponding rank. Errors herein will cause " << endl;
    cout << "connections to fail due to mismatches in portnumbering" << endl;
      
    return -1;
  } else {
    be_rank = atoi(argv[1]);
    cout << "be_rank = " << be_rank << endl;
  }

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
  const int targetgroups = kvm.getInt("targetgroups", 1);
  const bool blocking = kvm.getBool("blocking", true);

//   const std::string frontend_ip = kvm.getString("frontend_ip");
//   const std::string backend_ip = kvm.getString("backend_ip");
  
  const std::string loggerfile = kvm.getString("loggerfile", "CorrelatorLogger.prop");

  kvm.show(cout);

  if (be_rank >= targetgroups) {
    cout << "FrontEnd not used with this rank/targetgroups combination. " << endl;
    return 0;
  }

  INIT_LOGGER(loggerfile.c_str());

#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);
#endif

  for (int samples = min_samples; samples <= max_samples; samples=samples+stp_samples) {
    for (int elements = min_elements; elements <= max_elements; elements=elements+stp_elements) {
      
      try {
	AH_BackEnd* simulator;

	if ((samples+elements) % 2 == 0) {
	  simulator = new AH_BackEnd(port + targets + (targets/targetgroups)*be_rank, 
				     elements, 
				     samples, 
				     channels, 
				     polarisations, 
				     runs, 
				     targets,
				     targetgroups,
				     blocking);
	} else {
	  simulator = new AH_BackEnd((port+2*targets) + targets + (targets/targetgroups)*be_rank, 
				     elements, 
				     samples, 
				     channels, 
				     polarisations, 
				     runs, 
				     targets,
				     targetgroups,
				     blocking);
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
