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
		       char* ip, int baseport, int targets):
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels), 
  itsIP       (ip),
  itsBaseport (baseport),
  itsNtargets (targets)
{
}  

Correlator::~Correlator() {
}

void Correlator::define(const KeyValueMap& /*params*/) {

#ifdef HAVE_MPI
  sleep(TH_MPI::getCurrentRank());
  cout << "defining node number " << TH_MPI::getCurrentRank() << endl;
#endif


  // create the primary WorkHolder to do the actual work
  itsWH = (WorkHolder*) new WH_Correlator("noname",
					  itsNelements, 
					  itsNsamples,
					  itsNchannels, 
					  itsNtargets);
  itsWH->runOnNode(TH_MPI::getCurrentRank());
  
  // now create two dummy workholders to connect to
  // these will not exist outside the scope of this method
  WH_Random myWHRandom("noname",
		       itsNelements, 
		       itsNsamples,
		       itsNchannels);
  
  WH_Dump myWHDump("noname",
		   itsNelements,
		   itsNchannels);
  
  // now connect to the dummy workholders. 
#ifdef HAVE_MPI
  myWHRandom.getDataManager().getOutHolder(0)->connectTo 
    ( *itsWH->getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+TH_MPI::getCurrentRank(), true) );

  cout << "reading from port number: " << itsBaseport+TH_MPI::getCurrentRank() << " " << endl;

  itsWH->getDataManager().getOutHolder(0)->connectTo
    ( *myWHDump.getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+itsNtargets+TH_MPI::getCurrentRank(), false));
  cout << "writing to port number: " << itsBaseport+itsNtargets+TH_MPI::getCurrentRank() << endl;
#endif

}

void Correlator::undefine() {
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

int main (int argc, const char** argv) {

  //INIT_LOGGER("CorrelatorLogger.prop");
#ifdef HAVE_MPI
  TH_MPI::init(argc, argv);

  if (TH_MPI::getCurrentRank() < targets) {
#else 
  if (true) {
#endif

    for (int samples = min_samples; samples <= max_samples; samples++) {
      for (int elements = min_elements; elements <= max_elements; elements++) {
	
	// init the MPI environment.
	
	try {
	  
	  Correlator correlator(elements, samples, channels, frontend_ip, port, targets);
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

#ifdef HAVE_MPI
  // finalize the MPI environment
  TH_MPI::finalize();
#endif

  return 0;
}
