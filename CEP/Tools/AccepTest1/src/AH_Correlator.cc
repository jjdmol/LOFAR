//#  AH_Correlator.cc: Round robin correlator based on the premise that 
//#  BlueGene is a hard real-time system.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$

#include <AH_Correlator.h>

extern "C" void traceback (void);

using namespace LOFAR;

AH_Correlator::AH_Correlator(int elements, int samples, int channels, int polarisations, 
		       char* ip, int baseport, int targets):
  itsNelements(elements),
  itsNsamples (samples),
  itsNchannels(channels), 
  itsNpolarisations(polarisations),
  itsIP       (ip),
  itsBaseport (baseport),
  itsNtargets (targets)
{
}  

AH_Correlator::~AH_Correlator() {
}

void AH_Correlator::define(const KeyValueMap& /*params*/) {

#ifdef HAVE_MPI
  sleep(TH_MPI::getCurrentRank());
  cout << "defining node number " << TH_MPI::getCurrentRank() << endl;
#endif


  // create the primary WorkHolder to do the actual work
  itsWH = (WorkHolder*) new WH_Correlator("noname",
					  itsNelements, 
					  itsNsamples,
					  itsNchannels, 
					  itsNpolarisations,
					  itsNtargets);
#ifdef HAVE_MPI
  itsWH->runOnNode(TH_MPI::getCurrentRank());
#endif  

  // now create two dummy workholders to connect to
  // these will not exist outside the scope of this method
  WH_Random myWHRandom("noname",
		       itsNelements, 
		       itsNsamples,
		       itsNchannels,
		       itsNpolarisations);
  
  WH_Dump myWHDump("noname",
		   itsNelements,
		   itsNchannels,
		   itsNpolarisations);
  
  // now connect to the dummy workholders. 
#ifdef HAVE_MPI
  myWHRandom.getDataManager().getOutHolder(0)->connectTo 
    ( *itsWH->getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+TH_MPI::getCurrentRank(), false) );

  cout << "reading from port number: " << itsBaseport+TH_MPI::getCurrentRank() << " " << endl;

  itsWH->getDataManager().getOutHolder(0)->connectTo
    ( *myWHDump.getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+itsNtargets+TH_MPI::getCurrentRank(), true));
  cout << "writing to port number: " << itsBaseport+itsNtargets+TH_MPI::getCurrentRank() << endl;
#endif

}

void AH_Correlator::undefine() {
  delete itsWH;
}


void AH_Correlator::init() {
  itsWH->basePreprocess();
}

void AH_Correlator::run(int nsteps) {
  for (int i = 0; i < nsteps; i++) {
    itsWH->baseProcess();
  }
}

void AH_Correlator::dump () {
  itsWH->dump();
}

void AH_Correlator::postrun() {
  itsWH->basePostprocess();
}

void AH_Correlator::quit() {
}
