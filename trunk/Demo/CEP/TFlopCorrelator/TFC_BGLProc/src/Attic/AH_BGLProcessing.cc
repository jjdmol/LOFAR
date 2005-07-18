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

#include <APS/ParameterSet.h>

#include <AH_BGLProcessing.h>
#include <TFC_Interface/Stub_FIR.h>
#include <TFC_Interface/Stub_Corr.h>
// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <WH_FIR.h>
#include <WH_FFT.h>
#include <WH_Correlator.h>
// DataHolders
#include <TFC_Interface/DH_FIR.h>
#include <TFC_Interface/DH_CorrCube.h>
#include <TFC_Interface/DH_Vis.h>

using namespace LOFAR;

AH_BGLProcessing::AH_BGLProcessing() 
  : itsInStub (0),
    itsOutStub(0)
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
//   int itsNFIRF  = itsParamSet.getInt32("NFIRF");  // number of FIR filters in the application
  
  int lowestFreeNode = 0;
  
  LOG_TRACE_FLOW_STR("Create the top-level composite");

  // Create the bgl Processing section; these use  tinyCEP
  // The processing section consists of the FIR filter
  // and correlators

  LOG_TRACE_FLOW_STR("Create input side interface stubs");
  itsInStub = new Stub_FIR(true, itsParamSet);

  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  itsOutStub = new Stub_Corr(false, itsParamSet);

  LOG_TRACE_FLOW_STR("Create the FIR filter  workholders");
  
  char WH_Name[40];
  int noProcBlock = itsParamSet.getInt32("NoProcessingBlocks");
  int noFiltsPerBlock = itsParamSet.getInt32("NoFiltersPerBlock");
  int noCorsPerFilt = itsParamSet.getInt32("NoCorsPerFilt");
  int subband = 0;
  for (int pb = 0; pb < noProcBlock; pb++) {
    int corID = 0;
    vector<WH_Correlator*> Cors;
    for (int fil = 0; fil < noFiltsPerBlock; fil++) {
      // create WH_FIR and WH_FFT
      snprintf(WH_Name, 40, "FIRFilter_%d_of_%d_ofBlock_%d", fil, noFiltsPerBlock, pb);
      WH_FIR* FIRFNode = new WH_FIR(WH_Name, subband++);
      itsWHs.push_back(FIRFNode);
      itsWHs.back()->runOnNode(lowestFreeNode++);   

      snprintf(WH_Name, 40, "FFT_%d_of_%d_ofBlock_%d", fil, noFiltsPerBlock, pb);
      WH_FFT* FFTNode = new WH_FFT(WH_Name);
      itsWHs.push_back(FFTNode);
      itsWHs.back()->runOnNode(lowestFreeNode++);   

      // todo: connect to inputSection using Stub_FIR
      connectWHs(FIRFNode, 0, FFTNode, 0);
      
      for (int cor = 0; cor < noCorsPerFilt; cor++, corID++) {
	// create correlator nodes
	snprintf(WH_Name, 40, "Correlator_%d_of_%d_ofBlock_%d", corID, noFiltsPerBlock*noCorsPerFilt, pb);
	WH_Correlator* CorNode = new WH_Correlator(WH_Name);
	itsWHs.push_back(CorNode);
	itsWHs.back()->runOnNode(lowestFreeNode++);   
	Cors.push_back(CorNode);
	connectWHs(FFTNode, cor, CorNode, 0);
      }
    }

    // create collect node
    snprintf(WH_Name, 40, "Collect_ofBlock_%d", pb);
//     WH_Collect* ColNode = new WH_Collect(WH_Name);
//     itsWHs.push_back(ColNode);
//     itsWHs.back()->runOnNode(lowestFreeNode++);   
//     for (int cor = 0; cor <= corId; cor++) {
//       connectWHs(Cors[cor], 0, ColNode, cor);
//     }
    // todo: create a dummy (or more) so that the number of processes in this block
    // corresponds to the size of this block on BG/L (8 in CPM or 16 in VNM)
    
    // todo: connect to storage section using the Stub_Corr
  }
  
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
