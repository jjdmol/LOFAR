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
  itsNtargets (targets),
  itsRank(0)
{
#ifdef HAVE_MPI
  itsRank = TH_MPI::getCurrentRank();
#endif
}  

AH_Correlator::~AH_Correlator() {
}

void AH_Correlator::define(const KeyValueMap& /*params*/) {

  sleep(itsRank);


  // create the primary WorkHolder to do the actual work
  itsWH = (WorkHolder*) new WH_Correlator("noname",
					  itsNelements, 
					  itsNsamples,
					  itsNchannels, 
					  itsNpolarisations,
					  itsNtargets);
  itsWH->runOnNode(itsRank);

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

  myWHRandom.getDataManager().getOutHolder(0)->connectTo 
    ( *itsWH->getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+itsRank, false, true) );

  itsWH->getDataManager().getOutHolder(0)->connectTo
    ( *myWHDump.getDataManager().getInHolder(0), 
      TH_Socket(itsIP, itsIP, itsBaseport+itsNtargets+itsRank, true, true));
}

void AH_Correlator::undefine() {
  delete itsWH;
}


void AH_Correlator::init() {
  itsWH->basePreprocess();
}

void AH_Correlator::run(int nsteps) {
  double avg_bandwidth=0.0;
  double avg_corr_perf=0.0;
  
  for (int i = 0; i < nsteps; i++) {

#ifdef HAVE_MPE
    if (i != 0) MPE_Log_event(4, i, "transported");
#endif

    itsWH->baseProcess();
    
    if (itsRank == 0 && i > 0) {
      avg_bandwidth += static_cast<WH_Correlator*>(itsWH)->getAggBandwidth();
      avg_corr_perf += static_cast<WH_Correlator*>(itsWH)->getCorrPerf();
    }


#ifdef HAVE_MPE
    if (i < nsteps-1) MPE_Log_event(3, i, "transporting");
#endif

  }

  if (itsRank == 0) {
    cout << itsNelements << " " ;
    cout << itsNsamples  << " " ;
    cout << itsNchannels << " " ;
    cout << itsNpolarisations << " " ;
    cout << ((itsNchannels*itsNelements*itsNsamples*itsNpolarisations*sizeof(DH_CorrCube::BufferType)) + 
	     (itsNchannels*itsNelements*itsNelements*itsNpolarisations*sizeof(DH_Vis::BufferType)))/ (1024.0*1024.0) << " ";
    cout << avg_bandwidth/((nsteps-1)*1024.0*1024.0) << " Mbps " ;
    cout << (100.0*avg_bandwidth)/(nsteps*1024.0*1024.0*1024.0)<< "%" << " ";
    cout <<  avg_corr_perf/(nsteps-1) << " Mcprod/sec" << endl;
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
