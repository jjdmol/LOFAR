//#  FrontEnd.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$



// TransportHolders
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
//#include <Transport/TH_Socket.h>
#include "TH_Socket.h"
#include <tinyCEP/SimulatorParseClass.h>

#include <FrontEnd.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

FrontEnd::FrontEnd (bool frontend, int port, int elements, 
		    int samples, int channels, int runs, int targets):
  isFrontEnd  (frontend),
  itsPort     (port),
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels),
  itsNruns    (runs),
  itsNtargets (targets)
{
}


FrontEnd::~FrontEnd() {
  this->undefine();
}

void FrontEnd::define(const KeyValueMap& /*params*/) {

  undefine();

  WH_Correlator myWHCorrelator("noname",
			       1,
			       1, 
			       itsNelements, 
			       itsNsamples,
			       itsNchannels
			       );

  if (isFrontEnd) {

    for (int cn = 0; cn < itsNtargets; cn++) {

      itsWHs.push_back((WorkHolder*) 
		       new WH_Random("noname",
				     1, 
				     1, 
				     itsNelements,
				     itsNsamples,
				     itsNchannels));
      
      itsWHs.back()->getDataManager().getOutHolder(0)->connectTo
	( *myWHCorrelator.getDataManager().getInHolder(0),
	  TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+cn, true) );
      
    }
   
  } else {

    for (int cn = 0; cn < itsNtargets; cn++) {
  
      itsWHs.push_back((WorkHolder*)
		       new WH_Dump("noname",
				   1, 
				   1,
				   itsNelements, 
				   itsNchannels));
    
      myWHCorrelator.getDataManager().getOutHolder(0)->connectTo
	( *itsWHs.back()->getDataManager().getInHolder(0),
	  TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+itsNtargets+cn, false) );
    }
    
  } 
}

void FrontEnd::undefine() {

  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete (*it)->getDataManager().getInHolder(0)->getTransporter().getTransportHolder();
    delete (*it)->getDataManager().getOutHolder(0)->getTransporter().getTransportHolder();

  }
  itsWHs.clear();
}

void FrontEnd::init() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it != itsWHs.end(); it++) {
    (*it)->basePreprocess();
  }
}

void FrontEnd::run(int nsteps) {
  vector<WorkHolder*>::iterator it;

  for (int s = 0; s < nsteps; s++) {
    for (it = itsWHs.begin(); it != itsWHs.end(); it++) {
      (*it)->baseProcess();
    }
  }

}

void FrontEnd::dump() const {
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
//   for (; it != itsWHs.end(); it++) {
//     (*it)->dump();
//   }
}

void FrontEnd::quit() {
  this->undefine();
}

int parse_config() {
  int end = 0;

  config_file.open("config.dat");

  while (!config_file.eof()) {
    config_file.get(config_buffer[end++] );
  }
  config_file.close();

  Config params('=', 1, config_buffer);

  const char* ch = params("Correlator", "channels");
  if (ch) {
    nchannels = atoi(ch);
  }

  const char* el = params("Correlator", "elements");
  if (el) {
    nelements = atoi(el);
  }
  
  const char* rn = params("Correlator", "runs");
  if (rn) {
    nruns = atoi(rn);
  }

  const char* sa = params("Correlator", "samples");
  if (sa) {
    nsamples = atoi(sa);
  }
  
  const char* ip = params("Correlator", "frontendip");
  if (ip) {
    frontend_ip = (char*)malloc(15*sizeof(char));
    frontend_ip = strndup(ip, 15);
  }
  
  const char* pt = params("Correlator", "baseport");
  if (pt) {
    baseport = atoi(pt);
  }
   
  return 1;
}


int main (int argc, const char** argv) {

  bool isFrontEnd = true;

  INIT_LOGGER("CorrelatorLogger.prop");

  if (argc >= 2 && !strcmp(argv[1], "-b")) {
    
    isFrontEnd = false;
    
  }

  for (int samples = min_samples; samples <= max_samples; samples++) {
    for (int elements = min_elements; elements <= max_elements; elements++) {
      
      try {
		
	FrontEnd simulator(isFrontEnd, port, elements, samples,channels, runs, targets);
	
	simulator.setarg(argc, argv);
	simulator.baseDefine();
	simulator.basePrerun();
	simulator.baseRun(runs);
 	simulator.baseDump();
	simulator.baseQuit();

      } catch (LOFAR::Exception ex) {
	cout << "Caught a known exception" << endl;
	cout << ex.what() << endl;

      } catch (...) {
	cout << "Unexpected exception" << endl;
      }

    }
  }

  return 0;

}
