//#  CorrelatorMain: Main application for the correlatornodes
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <AH_Correlator.h>
#include <Common/KeyParser.h>
#include <Common/KeyValueMap.h>

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

  std::string frontend_ip = kvm.getString("frontend_ip", "192.168.100.31");
  std::string backend_ip = kvm.getString("backend_ip", "192.168.100.32");
  const std::string loggerfile = kvm.getString("loggerfile", "CorrelatorLogger.prop");


  INIT_LOGGER(loggerfile);

#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);

  if (TH_MPI::getCurrentRank() < targets) {
#else 
  if (true) {
#endif

#ifdef HAVE_MPE
    MPE_Init_log();

    MPE_Describe_state(1, 2, "Correlating","red");
    MPE_Describe_state(3, 4, "Transporting", "blue");
#endif 

    for (int samples = min_samples; samples <= max_samples; samples=samples+stp_samples) {
      for (int elements = min_elements; elements <= max_elements; elements=elements+stp_elements) {
	
	try {
	  
	  AH_Correlator* correlator;

	  if ((samples+elements) % 2 == 0) {
	    correlator = new AH_Correlator(elements, 
					   samples, 
					   channels, 
					   polarisations, 
					   const_cast<char*>(frontend_ip.c_str()), 
					   const_cast<char*>(backend_ip.c_str()), 
					   port, 
					   targets);
	  } else {
	    correlator = new AH_Correlator(elements, 
					   samples, 
					   channels, 
					   polarisations, 
					   const_cast<char*>(frontend_ip.c_str()), 
					   const_cast<char*>(backend_ip.c_str()), 
					   port+2*targets, 
					   targets);
	  }
	  
	  /* Automatic run of the correlator */
	  
	  correlator->baseDefine();
	  correlator->basePrerun();
	  
	  correlator->baseRun(runs);
	  
	  correlator->basePostrun();
	  // 	correlator.baseDump();
	  correlator->baseQuit();

	  delete correlator;

#ifdef HAVE_MPI
	  TH_MPI::synchroniseAllProcesses();
#endif

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

#ifdef HAVE_MPE
  MPE_Finish_log("correlator.log");
#endif

#ifdef HAVE_MPI
  // finalize the MPI environment
  TH_MPI::finalize();
#endif

  return 0;
}
