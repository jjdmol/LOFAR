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
#include <Common/KeyValueMap.h>
#include <Common/KeyParser.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <AH_FrontEnd.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

using namespace LOFAR;

int main (int argc, const char** argv) {

  int fe_rank = -1;

  if (argc != 2) {
    cout << "Usage " << argv[0] << " <FrontEnd rank>" << endl;
    cout << "For each targetgroup the user needs to start a FrontEnd" << endl;
    cout << "with the corresponding rank. Errors herein will cause " << endl;
    cout << "connections to fail due to mismatches in portnumbering" << endl;
      
    return -1;
  } else {
    fe_rank = atoi(argv[1]);
    cout << "fe_rank = " << fe_rank << endl;
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

//   const std::string frontend_ip = kvm.getString("frontend_ip");
//   const std::string backend_ip = kvm.getString("backend_ip");
  
  const std::string loggerfile = kvm.getString("loggerfile", "CorrelatorLogger.prop");

  if (fe_rank >= targetgroups) {
    cout << "FrontEnd not used with this rank/targetgroups combination. " << endl;
    return 0;
  }

  kvm.show(cout);
  INIT_LOGGER(loggerfile.c_str());


#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);
#endif

  for (int samples = min_samples; samples <= max_samples; samples=samples+stp_samples) {
    for (int elements = min_elements; elements <= max_elements; elements=elements+stp_elements) {
     
      try {		
	AH_FrontEnd* frontend;

	if ((samples+elements) % 2 == 0) {
	  frontend = new AH_FrontEnd(port + (targets/targetgroups)*fe_rank,
				     elements, 
				     samples, 
				     channels, 
				     polarisations, 
				     runs, 
				     targets, 
				     targetgroups);
	} else {
	  frontend = new AH_FrontEnd((port+2*targets) + (targets/targetgroups)*fe_rank, 
				     elements, 
				     samples, 
				     channels, 
				     polarisations, 
				     runs, 
				     targets, 
				     targetgroups);
	}
	

	frontend->setarg(argc, argv);
	frontend->baseDefine();
	frontend->basePrerun();	
	frontend->baseRun(runs);
 	frontend->baseDump();
	frontend->baseQuit();

	delete frontend;

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
