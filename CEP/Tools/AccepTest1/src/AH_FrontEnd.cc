//#  AH_FrontEnd.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <unistd.h>

#include <AH_FrontEnd.h>
// TransportHolders
#include <Common/lofar_iostream.h>
//#include <Common/LofarLogger.h>
#include <Common/KeyParser.h>
#include <Common/KeyValueMap.h>
#include <Transport/TH_Socket.h>
#include <tinyCEP/SimulatorParseClass.h>
#include <Transport/Connection.h>

#include <DH_CorrCube.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

AH_FrontEnd::AH_FrontEnd (int port, int elements, 
			  int samples, int channels, int polarisations, 
			  int runs, int targets, int targetgroups, bool blocking):
  itsPort     (port),
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels),
  itsNpolarisations(polarisations),
  itsNruns    (runs),
  itsNtargets (targets),
  itsNtgroups (targetgroups),
  itsBlocking (blocking)
{

  starttime.tv_sec = 0;
  starttime.tv_usec = 0;
  stoptime.tv_sec = 0;
  stoptime.tv_usec = 0;

  bandwidth = 0.0;

  gettimeofday (&starttime, NULL);
}


AH_FrontEnd::~AH_FrontEnd() {

  this->undefine();
}

void AH_FrontEnd::define(const KeyValueMap& /*params*/) {

  undefine();

  char name[32];
  for (int cn = 0; cn < itsNtargets/itsNtgroups; cn++) {
    sprintf(name,"FE_node%i",cn);
    itsWHs.push_back((WorkHolder*) 
		     new WH_Random(name,
				   itsNelements,
				   itsNsamples,
				   itsNchannels, 
				   itsNpolarisations));

    // create server socket
    string service(formatString("%d", itsPort+cn));
    itsOutTHs.push_back( new TH_Socket(service, itsBlocking) );
    itsOutConns.push_back ( new Connection("outConn",
				itsWHs.back()->getDataManager().getOutHolder(0),
				0,
				itsOutTHs.back()) );
    itsWHs.back()->getDataManager().setOutConnection(0, itsOutConns.back());

  }
}

void AH_FrontEnd::undefine() {
  vector<WorkHolder*>::iterator it1 = itsWHs.begin();
  for (; it1!=itsWHs.end(); it1++) {
    delete *it1;
  }
  itsWHs.clear();

  vector<Connection*>::iterator it2 = itsOutConns.begin();
  for (; it2!=itsOutConns.end(); it2++) {
    delete *it2;
  }
  itsOutConns.clear();

  vector<TransportHolder*>::iterator it3 = itsOutTHs.begin();
  for (; it3!=itsOutTHs.end(); it3++) {
    delete *it3;
  }
  itsOutTHs.clear();
}

//
// init()
//
// Start listeners on all WHs and wait for connection on all WHs
// 
void AH_FrontEnd::init() {
  // Iterate over workholder stack and start the listeners first
  int cn = 0;
  int good = 0;
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
  vector<TransportHolder*>::iterator iter = itsOutTHs.begin();
  cout << "init FE WH: starting all listeners..." << endl;
  for (; iter != itsOutTHs.end(); iter++)
  {
    if (!(*iter)->init())
    {
       cout << itsPort+cn << ":listener NOT OK" << endl;
    }
    else 
    {
      good++;
    }
    usleep(50);
    cn++;
  }


//   for (; it != itsWHs.end(); it++) {
// //    cout << "init FE WH " << (*it)->getName() << " listening on port " 
// //	 << itsPort+cn << endl;

//     // Get pointer to TH_Socket
//     TH_Socket* THS = static_cast<TH_Socket*>
// 		     ((*it)->getDataManager().getOutHolder(0)->getTransporter().getTransportHolder());
//     std::stringstream	service;	// construct string with portnumber
//     service << itsPort+cn;

//     // Start the listener
//     if (!THS->setListenSocket(new Socket("TH_Socket", service.str()))) {
// 	  cout << itsPort+cn << ":listener NOT OK" << endl;
//     }
//     else {
//       good++;
//     }

//     usleep(50);
//     cn++;
//   }
  

  cout << good << " of " << cn << " listeners started OK" << endl;

  // Now do the normal loop
  struct timeval timestamp;
  cn = 0;
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
    cout << "init FE WH " << (*it)->getName() << " accepting on port " 
         << itsPort+cn << endl;

    // do the accept
    (*it)->basePreprocess();

    // show the time it took us
    timestamp.tv_sec = 0;
    timestamp.tv_usec = 0;
    gettimeofday (&timestamp, NULL);
    cout << " connected on timestamp : "<< 
	    1.0 * (timestamp.tv_sec - starttime.tv_sec) + 
	    1.0 * (timestamp.tv_usec - starttime.tv_usec) / 1000000 
	 << "sec" << endl;
    cn++;
  }
  cout << "--> All correlators have connected" << endl;
}

void AH_FrontEnd::run(int nsteps) {
  vector<WorkHolder*>::iterator it;
  double aggregate_bandwidth=0.0;

  for (int s = 0; s < nsteps; s++) {
    for (it = itsWHs.begin(); it != itsWHs.end(); it++) {
      (*it)->baseProcess();
      aggregate_bandwidth += static_cast<WH_Random*>(*it)->getBandwidth();
    }
  }
//   if (aggregate_bandwidth != 0.0) {
//     cout << itsNelements << " " ;
//     cout << itsNsamples  << " " ;
//     cout << itsNchannels << " " ;
//     cout << itsNpolarisations << " " ;
//     cout << (8.0*aggregate_bandwidth)/(nsteps*1024.0*1024.0) << " Mbit/sec       ";
//     cout << (800.0*aggregate_bandwidth)/(nsteps*1024.0*1024.0*1024.0) << "% of theoretical peak (Gbit/sec)" << endl;
//     //     gettimeofday(&starttime, NULL);
//   }
}


void AH_FrontEnd::dump() const {
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
//   for (; it != itsWHs.end(); it++) {
//     (*it)->dump();
//   }
}

void AH_FrontEnd::quit() {
  this->undefine();
}
