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

  INIT_LOGGER("CorrelatorLogger");

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

#define NRFE 4 
  const int targetgroups = kvm.getInt("targetgroups", NRFE);

  std::string frontend_ip = kvm.getString("frontend_ip", "192.168.100.31");
  std::string backend_ip = kvm.getString("backend_ip", "192.168.100.32");
  //std::string loggerfile = kvm.getString("loggerfile", "CorrelatorLogger.prop");
  kvm.show(cout);


  //ASSERTSTR(targetgroups == NRFE,"Code not unrolled for other than 4 target groups yet..." );
  std::string FE_ip[NRFE];
  std::string BE_ip[NRFE];
  char f_ip[32];
  char b_ip[32];
  for (int n=0; n<NRFE; n++) { 
    sprintf(f_ip,"frontend_ip_%i",n);
    sprintf(b_ip,"backend_ip_%i",n);
    FE_ip[n] = kvm.getString(f_ip,frontend_ip.c_str());
    BE_ip[n] = kvm.getString(b_ip,backend_ip.c_str());
    cout << f_ip << " = " << FE_ip[n] << endl;
    cout << b_ip << " = " << BE_ip[n] << endl;
  }
  sleep(4);



  //INIT_LOGGER(loggerfile);
  
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

       	  char my_fe_ip[32];
       	  char my_be_ip[32];

#ifdef HAVE_MPI
          int rank = TH_MPI::getCurrentRank();
#else 
	  int rank = 0;
#endif

	  // logic for NRFE == 4
	  strcpy(my_fe_ip, FE_ip[0].c_str());
	  strcpy(my_be_ip, BE_ip[0].c_str());
	  if (rank < 3*targets/4) 
	    {
	      strcpy(my_fe_ip, FE_ip[3].c_str());
	      strcpy(my_be_ip, BE_ip[3].c_str());
	    } 
	  if (rank < 2*targets/4) 
	    {
	      strcpy(my_fe_ip, FE_ip[2].c_str());
	      strcpy(my_be_ip, BE_ip[2].c_str());
	    }
	  if (rank < 1*targets/4) 
	    {
	      strcpy(my_fe_ip, FE_ip[1].c_str());
	      strcpy(my_be_ip, BE_ip[1].c_str());
	    }
	
	  cout << "rank " << rank << "   my fe_ip " << my_fe_ip << "  my be_ip " << my_be_ip << endl;
	  int my_port = (((samples+elements) % 2 == 0) ? port : port+2*targets); 
	  //	    correlator = new AH_Correlator(elements, samples, channels, polarisations, fe_ip, backend_ip, port, targets);
	  correlator = new AH_Correlator(elements, 
					 samples, 
					 channels, 
					 polarisations, 
					 my_fe_ip, 
					 my_be_ip, 
					 my_port, 
					 targets);
	  
	  /* Automatic run of the correlator */
	  
	  correlator->baseDefine();
	  correlator->basePrerun();
	 
	  correlator->baseRun(runs);

#ifdef HAVE_MPI
	  TH_MPI::synchroniseAllProcesses();
#endif

	  correlator->basePostrun();
	  // 	correlator.baseDump();
	  correlator->baseQuit();

	  delete correlator;


#ifdef NOTDEF
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
