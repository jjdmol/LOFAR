//#  RTFrontEnd.cc:
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
#include <Transport/TH_Socket.h>
#include <tinyCEP/SimulatorParseClass.h>

#include <RTCorrelator/RTFrontEnd.h>

#define LOCALHOST_IP "127.0.0.1"


using namespace LOFAR;

RTFrontEnd::RTFrontEnd (bool frontend, int port, int elements, 
			int samples, int channels, int runs):
  isFrontEnd  (frontend),
  itsPort     (port),
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels),
  itsNruns    (runs)
{
}


RTFrontEnd::~RTFrontEnd() {
}

void RTFrontEnd::define(const KeyValueMap& /*params*/) {

  WH_Correlator myWHCorrelator("noname",
			       1,
			       1, 
			       itsNelements, 
			       itsNsamples,
			       itsNchannels, 
			       itsNruns     
			       );

  if (isFrontEnd) {
    
    itsWHs[0] = new WH_Random("noname",
			      1, 
			      1, 
			      itsNelements,
			      itsNsamples,
			      itsNchannels);
    itsWHs[0]->getDataManager().getOutHolder(0)->connectTo
      ( *myWHCorrelator.getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort, false) );

  } else {
	  
    itsWHs[0] = new WH_Dump("noname",
			    1, 
			    1,
			    itsNelements, 
			    itsNchannels);
    myWHCorrelator.getDataManager().getOutHolder(0)->connectTo
      ( *itsWHs[0]->getDataManager().getInHolder(0),
	TH_Socket(LOCALHOST_IP, LOCALHOST_IP, itsPort+1, true) );
    
  } 

  
  

}

void RTFrontEnd::init() {

  itsWHs[0]->basePreprocess();
//   itsWHs[1]->basePreprocess();

}

void RTFrontEnd::run(int nsteps) {
  
  for (int s = 0; s < nsteps; s++) {

    itsWHs[0]->baseProcess();
//     itsWHs[1]->baseProcess();

  }
}

void RTFrontEnd::dump() const {
  
  itsWHs[0]->dump();  
//   itsWHs[1]->dump();

}

void RTFrontEnd::quit() {
}

int parse_config() {
  int end = 0;

  config_file.open("config.dat");

  while (!config_file.eof()) {
    config_file.get(config_buffer[end++] );
  }
  config_file.close();

  Config params('=', 1, config_buffer);

  const char* ch = params("RTCorrelator", "channels");
  if (ch) {
    nchannels = atoi(ch);
  }

  const char* el = params("RTCorrelator", "elements");
  if (el) {
    nelements = atoi(el);
  }
  
  const char* rn = params("RTCorrelator", "runs");
  if (rn) {
    nruns = atoi(rn);
  }

  const char* sa = params("RTCorrelator", "samples");
  if (sa) {
    nsamples = atoi(sa);
  }
  
  const char* ip = params("RTCorrelator", "frontendip");
  if (ip) {
    frontend_ip = (char*)malloc(15*sizeof(char));
    frontend_ip = strndup(ip, 15);
  }
  
  const char* pt = params("RTCorrelator", "baseport");
  if (pt) {
    baseport = atoi(pt);
  }
   
  return 1;
}


int main (int argc, const char** argv) {

  bool isFrontEnd = true;

  if (!parse_config()) {
    cout << "Error reading config file" << endl;
  }

  try {

    INIT_LOGGER("RTLogger.prop");

    if (argc >= 2 && !strcmp(argv[1], "-b")) {

      isFrontEnd = false;

    }

    RTFrontEnd simulator(isFrontEnd, baseport, nelements, 
			 nsamples, nchannels, nruns);

    simulator.setarg(argc, argv);
    
    simulator.baseDefine();
    simulator.basePrerun();
    simulator.baseRun(nruns);
    simulator.baseDump();
    simulator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
}
