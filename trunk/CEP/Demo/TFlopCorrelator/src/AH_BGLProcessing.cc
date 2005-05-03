//#  AH_BGLProcessing.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/lofar_iostream.h>

#include <BGLProcessing/AH_BGLProcessing.h>

// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <CEPFrame/WH_Empty.h>
//#include <WH_SubBandFilter.h>
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
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}  

void AH_BGLProcessing::define() {


 LOG_TRACE_FLOW_STR("Start of AH_BGLProcessing::define()");

  LOG_TRACE_FLOW_STR("Create the top-level composite");

  // Create the bgl Processing section; these use  tinyCEP
  // The processing section consists of the SubBand filter
  // and correlators

  LOG_TRACE_FLOW_STR("Create input side interface stubs");
  SB_Stub inStub(false);

  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  Corr_Stub outStub(true);

  LOG_TRACE_FLOW_STR("Create the SubBand filter  workholders");
  vector<WH_SubBand*> SBFNodes;
  for (int i=0; i<NoSBFNodes; i++) {
    sprintf(WH_DH_Name, "SubBandFilter_%d_of_%d", i, NoSBFNodes);
    SBFNodes.push_back(new WH_SubBand(WH_DH_Name,      // name
				      0));   // SubBandID
    itsWHs.push_back((WorkHolder*) SBFNodes[i]);
    itsWHs[itsWHs.size()-1]->runOnNode(lowestFreeNode++);   

    // todo: connect SBFilter to input stub
    // this connection is defined in the SB_Stub class
    //inStub.connect (DH_SubBand& sb);
    
  };
  

  LOG_TRACE_FLOW_STR("Create the Correlator workholders");
  vector<WH_Correlator*> CorrNodes;
  for (int i=0; i<NoCorrNodes; i++) {
    sprintf(WH_DH_Name, "Correlator_%d_of_%d", i, NoCorrNodes);
    CorrNodes.push_back(new WH_Correlator(WH_DH_Name));      // name
    itsWHs.push_back((WorkHolder*) CorrNodes[i]);
    itsWHs[itsWHs.size()-1]->runOnNode(lowestFreeNode++);   

    // todo: connect Correlator to SBFilter
    // this connection is defined in the SB_Stub class

    // todo: connect correlator to output stub
    //outStub.connect();
  };
  
    
  LOG_TRACE_FLOW_STR("Finished define()");
}



void AH_BGLProcessing::prerun() {
  getComposite().preprocess();
}
    
void AH_BGLProcessing::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_BGLProcessing::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_BGLProcessing::run() "  );
}

void AH_BGLProcessing::dump() const {
  LOG_TRACE_FLOW_STR("AH_BGLProcessing::dump() not implemented"  );
}

void AH_BGLProcessing::quit() {

}


