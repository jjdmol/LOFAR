//#  AH_BackEnd.cc:
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
#include <Transport/TH_Socket.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <AH_BackEnd.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

AH_BackEnd::AH_BackEnd (int port, int elements, 
		  int samples, int channels, int polarisations, int runs, int targets):
  itsPort     (port),
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels),
  itsNpolarisations(polarisations),
  itsNruns    (runs),
  itsNtargets (targets)
{
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

  for (int cn = 0; cn < itsNtargets; cn++) {
    itsWHs.push_back((WorkHolder*)
		     new WH_Dump("noname",
				 itsNelements, 
				 itsNchannels,
				 itsNpolarisations));
    
    myWHCorrelator.getDataManager().getOutHolder(0)->connectTo
      ( *itsWHs.back()->getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+itsNtargets+cn, false) );
  }
}

void AH_BackEnd::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}

void AH_BackEnd::init() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
    (*it)->basePreprocess();
  }
}

void AH_BackEnd::run(int nsteps) {
  vector<WorkHolder*>::iterator it;

  for (int s = 0; s < nsteps; s++) {
    for (it = itsWHs.begin(); it != itsWHs.end(); it++) {
      (*it)->baseProcess();
    }
  }

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
