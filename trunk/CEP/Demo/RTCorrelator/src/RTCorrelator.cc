//#  RTCorrelator.cc: Round robin correlator based on the premise that 
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


#include <RTCorrelator.h>
#include <Transport/TH_MPI.h>

using namespace LOFAR;

RTCorrelator::RTCorrelator() {
}  

RTCorrelator::~RTCorrelator() {
}

void RTCorrelator::define(const KeyValueMap& /*params*/) {
  if (LOFAR::TH_MPI::getCurrentRank() == 0) {
    /* Create the master correlator and connect it to it's slaves and to */
    /* the outside world. The in- and outHolders with rank 0 will be the */
    /* socket connections to the outside, ranks > 0 will be the MPI      */
    /* connections to the slaves. The rank of the in- or outHolder       */ 
    /* corresponds to the MPI rank of the connected slave.               */

    itsWH = (WorkHolder*) new WH_Correlator("Master",
					    TH_MPI::getNumberOfNodes()+1, 
					    TH_MPI::getNumberOfNodes()+1,
					    nelements, nsamples, nchannels, nruns);
    
    WH_Correlator aSlave("noname", 1, 1, nelements, nsamples, nchannels, nruns);
    WH_Random     aRandom("noname", 0, 1, nelements, nsamples, nchannels);
    WH_Dump       aDump("noname", 1, 0);

    /* The correlator cannot accept connections because of the limitations   */
    /* of the BG/L socket implementation. Therefore all connections are      */
    /* opened from this application and accepted by the frontend application */
    TH_Socket fe_input(frontend_ip, frontend_ip, baseport, true);
    TH_Socket fe_output(frontend_ip, frontend_ip, baseport+1, false);

    aRandom.getDataManager().getOutHolder(0)->connectTo 
      ( *itsWH->getDataManager().getInHolder(0), 
	fe_input );
    
    itsWH->getDataManager().getOutHolder(0)->connectTo
      ( *aDump.getDataManager().getInHolder(0), 
	fe_output );

    for (int i = 1; i<TH_MPI::getNumberOfNodes(); i++) {
      /* [CHECKME] needs to be defined at global scope? */
      TH_MPI proto_input (0,i);
      TH_MPI proto_output(i,0);
	
      itsWH->getDataManager().getOutHolder(i)->connectTo
	( *aSlave.getDataManager().getInHolder(0),
	  proto_output );

      aSlave.getDataManager().getOutHolder(0)->connectTo
	( *itsWH->getDataManager().getInHolder(0),
	  proto_input );
    }
    
  } else {
    /* Create a slave correlator and connect it to it's master */
    itsWH = (WorkHolder*) new WH_Correlator("aSlave",
					    1, 
					    1,
					    nelements, nsamples, nchannels, nruns);
    /* Dummy workHolder to connect to */ 
    WH_Correlator myMaster("noname", 
			   TH_MPI::getNumberOfNodes()+1, 
			   TH_MPI::getNumberOfNodes()+1,
			   nelements, nsamples, nchannels, nruns);

    /* [CHECKME] Needs to be defined at global scope?? */
    TH_MPI proto_input(0, TH_MPI::getCurrentRank());
    TH_MPI proto_output(TH_MPI::getCurrentRank(), 0);

    myMaster.getDataManager().getOutHolder(TH_MPI::getCurrentRank())->connectTo
      ( *itsWH->getDataManager().getInHolder(0),
	proto_input);

    itsWH->getDataManager().getOutHolder(0)->connectTo
      ( *myMaster.getDataManager().getInHolder(TH_MPI::getCurrentRank()), 
	proto_output);
  }

}

void RTCorrelator::init() {
  itsWH->basePreprocess();
}

void RTCorrelator::run(int nsteps) {
  for (int i = 0; i < nsteps; i++) {
    itsWH->baseProcess();
  }
}

void RTCorrelator::dump () {
  itsWH->dump();
}

void RTCorrelator::postrun() {
  itsWH->basePostprocess();
}

void RTCorrelator::quit() {
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

  INIT_LOGGER("RTLogger.prop");

  LOFAR::TH_MPI::init(argc, argv);
  
  if (!parse_config()) {
    cout << "Error reading config file" << endl;
  }

  if (TH_MPI::getCurrentRank() == 0) {
    cout << "CHANNELS:   " << nchannels << endl;
    cout << "ELEMENTS:   " << nelements << endl;
    cout << "SAMPLES:    " << nsamples << endl;
    cout << "RUNS:       " << nruns << endl;
    cout << "FRONTEND:   " << frontend_ip << endl;
    cout << "BASEPORT:   " << baseport << endl;
  }

  RTCorrelator correlator;
  correlator.setarg(argc, argv);

#if 0
  /* Interactive run of the correlator */
  try {
    LOFAR::SimulatorParse::parse(correlator);
  } catch (LOFAR::SimulatorParseError x) {
    cout << x.what() << endl;
  }
#else
  /* Automatic run of the correlator based on parameters from config file */ 
  try {
    
    correlator.baseDefine();
    correlator.basePrerun();

    correlator.baseRun(nruns);
    
    correlator.basePostrun();
    correlator.baseDump();
    correlator.baseQuit();
  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
#endif



  LOFAR::TH_MPI::finalize();

  return 0;
}
