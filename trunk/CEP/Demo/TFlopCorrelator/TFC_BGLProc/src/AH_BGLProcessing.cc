//#  AH_BGLProcessing.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//
/////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/lofar_iostream.h>

#include <ACC/ParameterSet.h>

#include <AH_BGLProcessing.h>
#include <Stub_SB.h>
#include <Stub_Corr.h>
// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <WH_SubBand.h>
#include <WH_FFT.h>
#include <WH_Correlator.h>
// DataHolders
#include <DH_SubBand.h>
#include <DH_CorrCube.h>
#include <DH_Vis.h>

using namespace LOFAR;

AH_BGLProcessing::AH_BGLProcessing() 
{
}

AH_BGLProcessing::~AH_BGLProcessing() {
  this->undefine();
}

void AH_BGLProcessing::undefine() {
  vector<WorkHolder*>::iterator whit = itsWHs.begin();
  for (; whit!=itsWHs.end(); whit++) {
    delete *whit;
  }
  itsWHs.clear();
  vector<TransportHolder*>::iterator thit = itsTHs.begin();
  for (; thit!=itsTHs.end(); thit++) {
    delete *thit;
  }
  itsTHs.clear();
  vector<Connection*>::iterator cit = itsConnections.begin();
  for (; cit!=itsConnections.end(); cit++) {
    delete *cit;
  }
  itsConnections.clear();
}  

void AH_BGLProcessing::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_BGLProcessing::define()");
  int itsNSBF  = itsParamSet.getInt("NSBF");  // number of SubBand filters in the application
  
  int lowestFreeNode = 0;
  
  LOG_TRACE_FLOW_STR("Create the top-level composite");

  // Create the bgl Processing section; these use  tinyCEP
  // The processing section consists of the SubBand filter
  // and correlators

  LOG_TRACE_FLOW_STR("Create input side interface stubs");
  Stub_SB inStub(true);

  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  Stub_Corr outStub(false);

  LOG_TRACE_FLOW_STR("Create the SubBand filter  workholders");
  

  // create WH_SubBand and WH_FFT
  vector<WH_SubBand*> SBFNodes;
  vector<WH_FFT*> FFTNodes;
  char WH_DH_Name[40];
  for (int s=0; s<itsNSBF; s++) {
    snprintf(WH_DH_Name, 40, "SubBandFilter_%d_of_%d", s, itsNSBF);
    SBFNodes.push_back(new WH_SubBand(WH_DH_Name,      // name
				      0));   // SubBandID
    itsWHs.push_back((WorkHolder*) SBFNodes[s]);
    itsWHs[itsWHs.size()-1]->runOnNode(lowestFreeNode++);   

    // connect the Subband filter to the input section
    // this interface is defined in the Stub_SB class
    inStub.connect (s,                                                            // SBF filter number
		    (DH_SubBand*)SBFNodes[s]->getDataManager().getInHolder(0));  // input dataholder in the current WH



    snprintf(WH_DH_Name, 40, "SubBandFilter_FFT_%d_of_%d", s, itsNSBF);
    FFTNodes.push_back(new WH_FFT(WH_DH_Name));      // name
    itsWHs.push_back((WorkHolder*) FFTNodes[s]);
    itsWHs[itsWHs.size()-1]->runOnNode(lowestFreeNode++);   

    connectWHs(itsWHs[itsWHs.size()-2], 0, itsWHs[itsWHs.size()-1], 0);
  };
  
  LOG_TRACE_FLOW_STR("Create the Correlator workholders");
  vector<WH_Correlator*> CorrNodes;
  int corrID=0; // corr serial number in the AH
  int itsCpF   = itsParamSet.getInt("Corr_per_Filter");
  for (int s=0; s<itsNSBF; s++ ) {
    // loop over all SubBand Filters
    for (int c=0; c<itsCpF; c++) {
      // loop over al the correlators connected to a single SubBandFilter
      corrID++;  // 
      snprintf(WH_DH_Name, 40, "Correlator_%d_of_%d", corrID, itsNSBF*itsCpF);
      CorrNodes.push_back(new WH_Correlator(WH_DH_Name));      // name
      itsWHs.push_back((WorkHolder*) CorrNodes[corrID]);
      itsWHs[itsWHs.size()-1]->runOnNode(lowestFreeNode++);   
      
      // todo: connect Correlator to SBFilter
      
      // this connection is defined in the Stub_SB class
      
      // connect the Subband filter to the input section
      // this interface is defined in the Stub_SB class
      outStub.connect (corrID,                                                              // Corr filter number
		       *(DH_Vis*)CorrNodes[corrID]->getDataManager().getOutHolder(0));  // input dataholder in the current WH
      
    }
  };
  
  LOG_TRACE_FLOW_STR("Finished define()");
}



void AH_BGLProcessing::prerun() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it < itsWHs.end(); it++) {
    (*it)->basePreprocess();
  }
}
    
void AH_BGLProcessing::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_BGLProcessing::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (; it < itsWHs.end(); it++) {
      (*it)->baseProcess();
    }
  }
  LOG_TRACE_FLOW_STR("Finished AH_BGLProcessing::run() "  );
}

void AH_BGLProcessing::postrun() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it < itsWHs.end(); it++) {
    (*it)->basePostprocess();
  }
}


void AH_BGLProcessing::dump() const {
  vector<WorkHolder*>::const_iterator it;
  for ( it = itsWHs.begin(); it < itsWHs.end(); it++) {
    (*it)->dump();
  }
}

void AH_BGLProcessing::quit() {
  undefine();
}

void AH_BGLProcessing::connectWHs(WorkHolder* srcWH, int srcDH, WorkHolder* dstWH, int dstDH) {
#ifdef HAVE_MPI
  itsTHs.push_back(new TH_MPI(srcWH->getNode(), dstWH->getNode()) );
  itsConnections.push_back( new Connection("conn", 
					   srcWH->getDataManager().getOutHolder(srcDH),
					   dstWH->getDataManager().getInHolder(dstDH),
					   itsTHs.back(), true) );
#else
  itsTHs.push_back( new TH_Mem ); 
  itsConnections.push_back( new Connection("conn", 
					   srcWH->getDataManager().getOutHolder(srcDH),
					   dstWH->getDataManager().getInHolder(dstDH),
					   itsTHs.back(), false) );
#endif
  
  srcWH->getDataManager().setOutConnection(srcDH, itsConnections.back());
  dstWH->getDataManager().setInConnection(dstDH, itsConnections.back());
}
