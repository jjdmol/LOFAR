//#  CorrelatorMain: Main application for the correlatornodes
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#define MAX_TARGETGROUPS 4

#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <Common/KeyParser.h>
#include <Common/KeyValueMap.h>
#include <Common/Net/Socket.h>
#ifdef HAVE_MPI
#include <mpi.h>
#endif

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
  const int targets = kvm.getInt("targets", 1);

  int targetgroups = kvm.getInt("targetgroups", 1);

  std::string frontend_ip = kvm.getString("frontend_ip", "192.168.1.117");
  std::string backend_ip = kvm.getString("backend_ip", "192.168.1.117");
  std::string loggerfile = kvm.getString("loggerfile", "CorrelatorLogger");
//   kvm.show(cout);

  if (targetgroups > MAX_TARGETGROUPS) targetgroups=MAX_TARGETGROUPS;

  INIT_LOGGER(loggerfile.c_str());

  //ASSERTSTR(targetgroups == NRFE,"Code not unrolled for other than 4 target groups yet..." );
  std::string FE_ip[MAX_TARGETGROUPS];
  std::string BE_ip[MAX_TARGETGROUPS];
  char f_ip[32];
  char b_ip[32];
  for (int n=0; n<targetgroups; n++) { 
    sprintf(f_ip,"frontend_ip_%i",n);
    sprintf(b_ip,"backend_ip_%i",n);
    FE_ip[n] = kvm.getString(f_ip,frontend_ip.c_str());
    BE_ip[n] = kvm.getString(b_ip,backend_ip.c_str());
    cout << f_ip << " = " << FE_ip[n] << endl;
    cout << b_ip << " = " << BE_ip[n] << endl;
  }

  
#ifdef HAVE_MPI
  MPI_Init(&argc, (char***)&argv);
  int size;

  /// get the Number of nodes
  MPI_Comm_size (MPI_COMM_WORLD, &size);

  if (size == targets) {
#else 
  if (true) {
#endif

#ifdef HAVE_MPE
    MPE_Init_log();

    MPE_Describe_state(1, 2, "Correlating","red");
    MPE_Describe_state(3, 4, "Transporting", "blue");
#endif 

    for (int nrRuns=0; nrRuns<runs; nrRuns++) {	
	try {
	  
       	  char my_fe_ip[32];
       	  char my_be_ip[32];

#ifdef HAVE_MPI
	  int rank;

	  ///  Get the current node 
	  MPI_Comm_rank (MPI_COMM_WORLD, &rank);

#else 
	  int rank = 0;
#endif
	  if (targetgroups == 4) {
	    // logic for NRFE == 4
	    strcpy(my_fe_ip, FE_ip[3].c_str());
	    strcpy(my_be_ip, BE_ip[3].c_str());
	    if (rank < 3*targets/4) 
	      {
		strcpy(my_fe_ip, FE_ip[2].c_str());
		strcpy(my_be_ip, BE_ip[2].c_str());
	      } 
	    if (rank < 2*targets/4) 
	      {
		strcpy(my_fe_ip, FE_ip[1].c_str());
		strcpy(my_be_ip, BE_ip[1].c_str());
	      }
	    if (rank < 1*targets/4) 
	      {
		strcpy(my_fe_ip, FE_ip[0].c_str());
		strcpy(my_be_ip, BE_ip[0].c_str());
	      }
	  } else if (targetgroups == 1) {
	    strcpy(my_fe_ip,FE_ip[targetgroups-1].c_str());
	    strcpy(my_be_ip,BE_ip[targetgroups-1].c_str());
	  } else {
	    cout << "targetgroups should either be 1 or 4. You are using targetgroup = " << targetgroups << endl;
	    return -1;
	  }
	
	  cout << "rank " << rank << "   my fe_ip " << my_fe_ip << "  my be_ip " << my_be_ip << endl;
 	  int my_port = (((nrRuns) % 2 == 0) ? port : port+2*targets); 
//	  int my_port = port;
	  //	    correlator = new AH_Correlator(elements, samples, channels, polarisations, fe_ip, backend_ip, port, targets);

	  // create new data socket for Front End connection
	  int fe_port = my_port + rank;
	  std::stringstream service;
	  service <<  fe_port;
	  Socket* fe_dataSocket  = new Socket("TH_Socket", my_fe_ip, service.str());

	  fe_dataSocket->setBlocking(true);
	 
	  ASSERTSTR (fe_dataSocket->ok(), "Start connect to front end server (" <<
		     my_fe_ip << "," <<
		     fe_port << ") failed: " <<
		     fe_dataSocket->errstr());

	  // try to connect to front end
	  int nrTry = 60;
	  do
	  {
	    fe_dataSocket->connect(-1);
	    if (fe_dataSocket->isConnected())
	    {
	      LOG_TRACE_FLOW_STR("Correlator " << rank << " connected succesfully to front end server"); 
      	      break;
	    }
	    nrTry--;
	  }
	  while (nrTry>=0);
	  ASSERTSTR((fe_dataSocket->isConnected()), "Correlator " << rank << " connect to front end server" 
		    <<  my_fe_ip << ", on port " << fe_port << " FAILED");


	  // create new data socket for Back End connection
	  int be_port = my_port + targets + rank;
	  std::stringstream serv;
	  serv <<  be_port;
	  Socket* be_dataSocket  = new Socket("TH_Socket", my_be_ip, serv.str());

	  be_dataSocket->setBlocking(true);

	  ASSERTSTR (be_dataSocket->ok(), "TH_Socket::connect to front end server (" <<
		     my_be_ip << "," <<
		     be_port << ") failed: " <<
		     be_dataSocket->errstr());

	  // try to connect to back end
	  nrTry = 60;
	  do
	  {
	    be_dataSocket->connect(-1);
	    if (be_dataSocket->isConnected())
	    {
	      LOG_TRACE_FLOW_STR("Correlator " << rank << " connected succesfully to back end server"); 
	      break;
	    }
	    nrTry--;
	  }
	  while (nrTry>=0);
	  ASSERTSTR((be_dataSocket->isConnected()), "Correlator " << rank << " connect to back end server" 
		    <<  my_be_ip << ", on port " << be_port << " FAILED");



#ifdef HAVE_MPI
	  MPI_Barrier(MPI_COMM_WORLD);
#endif

	  sleep(2);

	  delete fe_dataSocket;
	  delete be_dataSocket;


#ifdef NOTDEF
	  MPI_Barrier(MPI_COMM_WORLD);
#endif

	} catch (LOFAR::Exception ex) {
	  // catch known exceptions
	  cout << "Caught a known exception" << endl;
	  cout << ex.what() << endl;
	  
	} catch (...) {
	  
	  cout << "Unexpected exception" << endl;
	  
	}
    }

  } else {
#ifdef HAVE_BGL
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    if (rank == 0) cout << "ERROR: MPI size does not match the number of targets. Restart with BLGMPI_SIZE=" << targets << endl;
#endif 
#ifdef HAVE_MPI
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    if (rank == 0) cout << "ERROR: MPI size does not match the number of targets. Restart with -np " << targets << endl;
#endif
  }

#ifdef HAVE_MPE
  MPE_Finish_log("correlator.log");
#endif

#ifdef HAVE_MPI
  // finalize the MPI environment
  MPI_Finalize();
#endif

  return 0;
}
