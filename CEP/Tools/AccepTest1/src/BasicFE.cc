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
#include <Common/Net/Socket.h>

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
  const bool blocking = kvm.getBool("blocking", true);


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

  for (int nrRuns=0; nrRuns<runs; nrRuns++) {	
    
      int nrSockets = targets;
      int good=0;
      int listenercnt=0;
      int acceptcnt=0;
      int thePort=0;
      
      try 
      { 
	if ((nrRuns) % 2 == 0) {
	  thePort = port + (targets/targetgroups)*fe_rank;
	} else {
	  thePort = (port+2*targets) + (targets/targetgroups)*fe_rank; 
	}
	//	thePort = port;

	// Start listeners
	Socket* socketList[nrSockets];
	for (int i=0; i<nrSockets; i++)
	{
	  std::stringstream  service;	// construct string with portnumber
	  int portnr = thePort+listenercnt;
	  service << portnr;
	  
	  // Start a listener
	  socketList[i] = new Socket("TH_Socket", service.str());    //

	  if (socketList[i]==0 || !(socketList[i]->isServer()) || !(socketList[i]->ok())) 
	  {
	    cout << portnr << ":listener NOT OK" << endl;
	  }
	  else
	  {
	    good++;
	  }
	  usleep(50);
	  listenercnt++; 
	}      
	cout << good << " of " << listenercnt << " listeners started OK" << endl;

	
	// Start accepting
	Socket* dataSockList[nrSockets];
	for (int i=0; i<nrSockets; i++)
	{
	  cout << "Basic FE " << " accepting on port " << thePort+acceptcnt << endl;
	  // Accept a connection

	  dataSockList[i] = 0;
	  dataSockList[i] = socketList[i]->accept(-1);

	  if (dataSockList[i]==0) 
          {
	    cout << port+acceptcnt << ": accepting NOT OK. Data socket ptr is 0" << endl;	    
	  }
	  else if (!(dataSockList[i]->isConnected()))
	  {
	    cout << port+acceptcnt << ": accepting NOT OK. Data socket is not connected." << endl;
	  }
// 	  else if (!(dataSockList[i]->ok()))
// 	  {
// 	    int32 errcode = dataSockList[i]->errcode();
// 	    cout << port+acceptcnt << ": accepting NOT OK with errorcode " <<
// 		 errcode << endl;
// 	  }
	  acceptcnt++;
	}      

	sleep(2);

	for (int j=0; j<nrSockets; j++)
	{
	  delete socketList[j];
	  delete dataSockList[j];
	}
      }
      catch (LOFAR::Exception ex) {
	cout << "Caught a known exception" << endl;
	cout << ex.what() << endl;
	
      } catch (...) {
	cout << "Unexpected exception" << endl;
      }  
  }

#ifdef HAVE_MPI
  TH_MPI::finalize();
#endif

  return 0;

}
