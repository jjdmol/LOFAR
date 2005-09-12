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
#include <Transport/TH_Socket.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <WH_PPF.h>
#include <WH_Correlator.h>
// DataHolders
#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>
#include <TFC_Interface/DH_Vis.h>

using namespace LOFAR;

AH_BGLProcessing::AH_BGLProcessing() 
  : itsWHs(0),
    itsConnections(0),
    itsTHs(0)
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

  int lowestFreeNode = 0;
  
  LOG_TRACE_FLOW_STR("Create the top-level composite");

  // Create the bgl Processing section; these use  tinyCEP
  // The processing section consists of the FIR filter
  // and correlators

  LOG_TRACE_FLOW_STR("Create input side interface stubs");
  itsInStub = new Stub_FIR(false, itsParamSet);

  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  itsOutStub = new Stub_Corr(false, itsParamSet);

//   LOG_TRACE_FLOW_STR("Create the FIR filter  workholders");
  
  char WH_Name[40];
//   int noProcBlock = itsParamSet.getInt32("NoProcessingBlocks");
//   int noFiltsPerBlock = itsParamSet.getInt32("NoFiltersPerBlock");
//   int subband = 0;

  int noCorsPerFilt = itsParamSet.getInt32("Input.NSubbands");

//   int InputBasePort = itsParamSet.getInt32("FIRConnection.RequestPort");
//   string itsInServer = itsParamSet.getString("FIRConnection.ServerHost");
// //   string itsInServer = itsParamSet.getString("InServer");

//   int OutputBasePort = itsParamSet.getInt32("CorrConnection.RequestPort");
//   string itsOutServer = itsParamSet.getString("CorrConnection.ServerHost");

//   int itsBasePort = itsParamSet.getInt32("BasePort");
//   string itsOutServer = itsParamSet.getString("OutServer");

  // define a block of correlators

  for (int cor = 0; cor < noCorsPerFilt; cor++) {

    snprintf(WH_Name, 40, "Correlator_%d_of_%d", cor, noCorsPerFilt);
    WH_Correlator* CorNode = new WH_Correlator(WH_Name, NR_CHANNELS_PER_CORRELATOR);
    itsWHs.push_back(CorNode);
    itsWHs.back()->runOnNode(lowestFreeNode++);

    itsInStub->connect(cor, itsWHs.back()->getDataManager(), 0);
    // for the first test, don't connect the output
    itsOutStub->connect(cor, itsWHs.back()->getDataManager(), 0);

//     DataHolder* itsInDH = new DH_FIR("itsIn1", 0, itsParamSet);
//     DataHolder* itsOutDH = new DH_Vis("itsOut1", 0, itsParamSet);
        
//     string itsInService(formatString("%d", itsBasePort+2*cor));
//     string itsOutService(formatString("%d", itsBasePort+2*cor+1));

//     TransportHolder* itsInTH = new TH_Mem();
//     TransportHolder* itsInTH = new TH_Socket(itsInServer, itsInService);
//     itsTHs.push_back(itsInTH);
    
//     Connection* itsInConnection = new Connection("itsInCon",
// 						 0,
// 						 itsWHs.back()->getDataManager().getInHolder(0),
// 						 itsTHs.back(),
// 						 true); // connection is blocking
//     itsConnections.push_back(itsInConnection);

//     TransportHolder* itsOutTH = new TH_Mem();
//     TransportHolder* itsOutTH = new TH_Socket(itsOutServer, itsOutService);
//     itsTHs.push_back(itsOutTH);
    
//     Connection* itsOutConnection = new Connection("itsOutCon",
// 						  itsWHs.back()->getDataManager().getOutHolder(0),
// 						  0,
// 						  itsTHs.back(), 
// 						  true); // connection is blocking
//     itsConnections.push_back(itsOutConnection);
  }

#ifdef HAVE_MPI
  ASSERTSTR (lowestFreeNode == TH_MPI::getNumberOfNodes(), "TFC_BGLProc needs " << lowestFreeNode << " nodes, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif
  
  LOG_TRACE_FLOW_STR("Finished define()");
}

void AH_BGLProcessing::init() {
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

// void AH_BGLProcessing::postrun() {
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
//   for (; it < itsWHs.end(); it++) {
//     (*it)->basePostprocess();
//   }
// }


void AH_BGLProcessing::dump() const {
  vector<WorkHolder*>::const_iterator it;
  for ( it = itsWHs.begin(); it < itsWHs.end(); it++) {
#ifdef HAVE_MPI
    if ((*it)->getNode() == TH_MPI::getCurrentRank()) {
      (*it)->dump();
    }
#else
    (*it)->dump();
#endif
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
