//#  Correlator.cc: Round robin correlator based on the premise that 
//#  BlueGene is a hard real-time system.
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

#include <Correlator.h>

extern "C" void traceback (void);

using namespace LOFAR;

Correlator::Correlator(int elements, int samples, int channels, 
		       char* ip, int baseport):
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels), 
  itsIP       (ip),
  itsBaseport (baseport)
{
}  

Correlator::~Correlator() {
}

void Correlator::define(const KeyValueMap& /*params*/) {

  // create the primary WorkHolder to do the actual work
  itsWH = (WorkHolder*) new WH_Correlator("noname",
					  1, 
					  1, 
					  itsNelements, 
					  itsNsamples,
					  itsNchannels);
  
  // now create two dummy workholders to connect to
  // these will not exist outside the scope of this method
  WH_Random myWHRandom("noname",
		       1, 
		       1, 
		       itsNelements, 
		       itsNsamples,
		       itsNchannels);
  
  WH_Dump myWHDump("noname",
		   1, 
		   1, 
		   itsNelements,
		   itsNchannels);
  
  // now connect to the dummy workholders. 
  myWHRandom.getDataManager().getOutHolder(0)->connectTo 
    ( *itsWH->getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+TH_MPI::getCurrentRank(), true) );
  
  itsWH->getDataManager().getOutHolder(0)->connectTo
    ( *myWHDump.getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+TH_MPI::getNumberOfNodes()+TH_MPI::getCurrentRank(), false));
}

void Correlator::undefine() {
  delete itsWH->getDataManager().getInHolder(0)->getTransporter().getTransportHolder();
  delete itsWH->getDataManager().getOutHolder(0)->getTransporter().getTransportHolder();

  delete itsWH;
}


void Correlator::init() {
  itsWH->basePreprocess();
}

void Correlator::run(int nsteps) {
  for (int i = 0; i < nsteps; i++) {
    itsWH->baseProcess();
  }
}

void Correlator::dump () {
  itsWH->dump();
}

void Correlator::postrun() {
  itsWH->basePostprocess();
}

void Correlator::quit() {
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

  INIT_LOGGER("CorrelatorLogger.prop");
  TH_MPI::init(argc, argv);

  if (TH_MPI::getCurrentRank() < targets) {

    for (int samples = min_samples; samples <= max_samples; samples++) {
      for (int elements = min_elements; elements <= max_elements; elements++) {
	
	// init the MPI environment.
	
	try {
	  
	  Correlator correlator(elements, samples, channels, frontend_ip, port);
	  correlator.setarg(argc, argv);
	  
	  /* Automatic run of the correlator */
	  
	  correlator.baseDefine();
	  correlator.basePrerun();
	  
	  correlator.baseRun(runs);
	  
	  correlator.basePostrun();
	  // 	correlator.baseDump();
	  correlator.baseQuit();
	  
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

  // finalize the MPI environment
  TH_MPI::finalize();

  return 0;
}
