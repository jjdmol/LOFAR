//#  AH_BackEnd.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <sys/time.h>

// TransportHolders
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <Transport/TH_Socket.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <DH_Vis.h>
#include <AH_BackEnd.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

AH_BackEnd::AH_BackEnd (int port, int elements, 
			int samples, int channels, 
			int polarisations, int runs, 
			int targets, int targetgroups,
			bool blocking
			):
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


AH_BackEnd::~AH_BackEnd() {
  this->undefine();
}

void AH_BackEnd::define(const KeyValueMap& /*params*/) {

  undefine();

  WH_Correlator myWHCorrelator("noname",
			       itsNelements, 
			       itsNsamples,
			       itsNchannels, 
			       itsNtargets,
			       itsNpolarisations
			       );

  for (int cn = 0; cn < itsNtargets/itsNtgroups; cn++) {
    itsWHs.push_back((WorkHolder*)
		     new WH_Dump("noname",
				 itsNelements, 
				 itsNchannels,
				 itsNpolarisations));

    myWHCorrelator.getDataManager().getOutHolder(0)->connectTo
      ( *itsWHs.back()->getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+cn, true, itsBlocking) );
  }
}

void AH_BackEnd::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}

//
// init()
//
// Start listeners on all WHs and wait for connection on all WHs
//
void AH_BackEnd::init() {
  // Iterate over workholder stack and start the listeners first
  int cn = 0;
  int good = 0;
  cout << "init BE WH: starting all listeners..." << endl;
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
//    cout << "init BE WH " << (*it)->getName() << " listening on port " 
//         << itsPort+cn << endl;

    // Get pointer to TH_Socket
    TH_Socket* THS = static_cast<TH_Socket*>
                     ((*it)->getDataManager().getInHolder(0)->getTransporter().getTransportHolder());
    std::stringstream	service;	// construct string with portnumber
    service << itsPort+cn;
    // Start the listener first
    if (!THS->setListenSocket(new Socket("TH_Socket", service.str()))) {
      cout << itsPort+cn << ":listener NOT OK" << endl;
    }
    else {
      good++;
    }

    usleep(50);
    cn++;
  }
  cout << good << " of " << cn << " listeners started OK" << endl;

  // Now do the normal loop
  struct timeval timestamp;
  cn = 0;
  it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
    cout << "init BE WH " << (*it)->getName() << " accepting on port " 
         << itsPort+cn << endl;

    // be sure right blocking mode is used.
    (*it)->getDataManager().getInHolder(0)->getTransporter().setIsBlocking(itsBlocking);
    // do the accept
    (*it)->basePreprocess();

    // show the time it took us
    gettimeofday (&timestamp, NULL);
    cout << " connected on timestamp : "<< 
            1.0 * (timestamp.tv_sec - starttime.tv_sec) + 
            1.0 * (timestamp.tv_usec - starttime.tv_usec) / 1000000 
            << "sec" << endl;
    cn++;
  }
}

void AH_BackEnd::run(int nsteps) {
  vector<WorkHolder*>::iterator it;
  double aggregate_bandwidth=0.0;
  
  for (int s = 0; s < nsteps; s++) {

    for (it = itsWHs.begin(); it != itsWHs.end(); it++) {
      (*it)->baseProcess();
      aggregate_bandwidth += reinterpret_cast<WH_Dump*> (*it)->getBandwidth();
    }
  }

//   if (aggregate_bandwidth != 0.0) {
//     cout << itsNelements << " " ;
//     cout << itsNsamples  << " " ;
//     cout << itsNchannels << " " ;
//     cout << itsNpolarisations << " " ;
//     cout << (8.0*aggregate_bandwidth)/(nsteps*1024.0*1024.0) << " Mbit/sec       ";
//     cout << (800.0*aggregate_bandwidth)/(nsteps*1024.0*1024.0*1024.0) << "% of theoretical peak (Gbit/sec)" << endl;
//   }
}

void AH_BackEnd::dump() const {
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
//   for (; it != itsWHs.end(); it++) {
//     (*it)->dump();
//   }
}

void AH_BackEnd::quit() {
  this->undefine();
}
