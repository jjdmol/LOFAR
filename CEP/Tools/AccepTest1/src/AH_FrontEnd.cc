//#  AH_FrontEnd.cc:
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

#include <AH_FrontEnd.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

AH_FrontEnd::AH_FrontEnd (int port, int elements, 
		    int samples, int channels, int runs, int targets):
  itsPort     (port),
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels),
  itsNruns    (runs),
  itsNtargets (targets)
{
}


AH_FrontEnd::~AH_FrontEnd() {
  this->undefine();
}

void AH_FrontEnd::define(const KeyValueMap& /*params*/) {

  undefine();

  WH_Correlator myWHCorrelator("noname",
			       itsNelements, 
			       itsNsamples,
			       itsNchannels, 
			       itsNtargets
			       );

  for (int cn = 0; cn < itsNtargets; cn++) {
    itsWHs.push_back((WorkHolder*) 
		     new WH_Random("noname",
				   itsNelements,
				   itsNsamples,
				   itsNchannels));
      
    itsWHs.back()->getDataManager().getOutHolder(0)->connectTo
      ( *myWHCorrelator.getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+cn, true) );
    
  }
}

void AH_FrontEnd::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}

void AH_FrontEnd::init() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
    (*it)->basePreprocess();
  }
}

void AH_FrontEnd::run(int nsteps) {
  vector<WorkHolder*>::iterator it;

  for (int s = 0; s < nsteps; s++) {
    for (it = itsWHs.begin(); it != itsWHs.end(); it++) {
      (*it)->baseProcess();
    }
  }

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
