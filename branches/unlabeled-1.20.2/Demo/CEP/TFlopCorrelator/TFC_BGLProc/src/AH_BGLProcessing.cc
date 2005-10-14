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
#include <TFC_Interface/TFC_Config.h>
// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
#include <Transport/TH_Socket.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
// #include <WH_BGL_Processing.h>
#include <WH_Distribute.h>
#include <WH_PPF.h>
#include <WH_Correlator.h>
#include <WH_Concentrator.h>
// DataHolders
#include <TFC_Interface/DH_Subband.h>
#include <TFC_Interface/DH_PPF.h>
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

  LOG_TRACE_FLOW_STR("Create the Polyphase filter workholders");
  
  char WH_Name[40];

  int itsIn = 0;
  int itsOut = 0;

  int itsNrComputeCells = itsParamSet.getInt32("BGLProc.NrComputeCells");
  int itsNrFilters = itsParamSet.getInt32("BGLProc.NrFiltersPerComputeCell");
  int itsNrFiltersPerComputeCell = itsParamSet.getInt32("BGLProc.NrFiltersPerComputeCell");
  int itsNrCorrelatorsPerFilter = itsParamSet.getInt32("PPF.NrCorrelatorsPerFilter");
  int itsNrChannels = itsParamSet.getInt32("PPF.NrSubChannels");
  int itsNrVisPerInput = itsParamSet.getInt32("Storage.NVisPerInput");
  int itsNrStoredSubbands = itsParamSet.getInt32("BGLProc.NrStoredSubbands");
  int itsNSBCollectOutputs = itsParamSet.getInt32("Input.NSBCollectOutputs");
  
  vector<int> itsInputPorts      = itsParamSet.getInt32Vector("FIRConnection.RequestPorts");
  vector<string> itsInputServers = itsParamSet.getStringVector("FIRConnection.ServerHosts");

  vector<int> itsOutputPorts      = itsParamSet.getInt32Vector("CorrConnection.RequestPorts");
  vector<string> itsOutputServers = itsParamSet.getStringVector("CorrConnection.ServerHosts");

  
  for (int ComputeCells = 0; ComputeCells < itsNrComputeCells; ComputeCells++) {
    WH_Distribute*  DistNode;
    WH_Concentrator* ConcNode;

    vector<WH_PPF*> PPFNodes;
    vector<WH_Correlator*> CorrNodes;

    // The basic definition of a "compute cell"
    // A compute cell is a connected set of processing blocks that implement 
    // a complete poly-phase filter and correlator chain. Each compute cell 
    // contains a single WH_Distribute that spreads the input to a number of
    // processing chains.
    DistNode = new WH_Distribute("distribute", itsParamSet, 1, 2*itsNrFiltersPerComputeCell);
    itsWHs.push_back(DistNode);
    itsWHs.back()->runOnNode(lowestFreeNode++);

    itsInStub->connect(itsIn++, itsWHs.back()->getDataManager(), 0);
    
    /* While only a single compute block of 2 filters and 4 correlators would suffice  */
    /* within a compute cell, this would leave more than 50% of the cell unused. To    */
    /* reach the desired 1 TFlop of computational load, we duplicate the compute block */
    /* even though this calculates exactly the same.                                   */

    for (int i = 0; i < 2; i++) {

      for (int Filters = 0; Filters < itsNrFiltersPerComputeCell; Filters++) {
	snprintf(WH_Name, 40, "PPF_%d_of_%d", Filters, itsNrFilters);
	PPFNodes.push_back(new WH_PPF(WH_Name, 0, 19));
	itsWHs.push_back(PPFNodes.back());
	itsWHs.back()->runOnNode(lowestFreeNode++);
      }

      for (int Correlators = 0; Correlators < itsNrCorrelatorsPerFilter; Correlators++) {
	snprintf(WH_Name, 40, "CORR_%d_of_%d", Correlators, itsNrCorrelatorsPerFilter);
	CorrNodes.push_back(new WH_Correlator(WH_Name));
	itsWHs.push_back(CorrNodes.back());
	itsWHs.back()->runOnNode(lowestFreeNode++);
      }

      // Now connect the internal blocks together
      int filter_nr = 0;
      vector<WH_PPF*>::iterator fit = PPFNodes.begin();
      for (; fit != PPFNodes.end(); fit++) {
	// connect the distribute node to it's filters
	itsTHs.push_back(new TH_MPI( DistNode->getNode(), (*fit)->getNode() ));
	itsConnections.push_back(new Connection("dist_conn",
						DistNode->getDataManager().getOutHolder(filter_nr),
						(*fit)->getDataManager().getInHolder(0),
						itsTHs.back(),
						false)); 


	int corr_nr   = 0;
	vector<WH_Correlator*>::iterator cit = CorrNodes.begin();

 	for (; cit != CorrNodes.end(); cit++) {
 	  snprintf(WH_Name, 40, "conn_filter_%d_corr_%d", filter_nr, corr_nr);
 	  itsTHs.push_back( new TH_MPI( (*fit)->getNode(), (*cit)->getNode() ) );
	  itsConnections.push_back(new Connection(WH_Name, 
						  (*fit)->getDataManager().getOutHolder(corr_nr % itsNrCorrelatorsPerFilter),
						  (*cit)->getDataManager().getInHolder(filter_nr % itsNrFiltersPerComputeCell),
						  itsTHs.back(),
						  false));
	  corr_nr++;
	}
	filter_nr++;
      }
    }

    ConcNode = new WH_Concentrator("concentrator", itsParamSet, itsNrVisPerInput);
    itsWHs.push_back(ConcNode);
    itsWHs.back()->runOnNode(lowestFreeNode++);

    vector<WH_Correlator*>::iterator cit = CorrNodes.begin();
    int corr_nr = 0;
    for (; cit != CorrNodes.end(); cit++) {
      itsTHs.push_back( new TH_MPI( (*cit)->getNode(), ConcNode->getNode() ) );
      itsConnections.push_back(new Connection(WH_Name, 
					      (*cit)->getDataManager().getOutHolder(0),
					      ConcNode->getDataManager().getInHolder(corr_nr++),
					      itsTHs.back(),
					      false));
    }
   
    // We only connect every 16th computecell to the outside world.
    // The reason for this is that we expect every 16th subband to be unique.
    if (ComputeCells % itsNSBCollectOutputs == 0) {
      itsInStub->connect(itsIn++, itsWHs.back()->getDataManager(), 0);
    }
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
