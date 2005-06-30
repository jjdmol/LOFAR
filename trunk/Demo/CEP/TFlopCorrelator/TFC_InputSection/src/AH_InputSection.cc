//#  AH_InputSection.cc: 
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

#include <AH_InputSection.h>

// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <WH_RSPInput.h>
#include <WH_Transpose.h>
#include <WH_SyncControl.h>
#include <TFC_Interface/Stub_FIR.h>

// DataHolders
#include <TFC_Interface/DH_SubBand.h>

using namespace LOFAR;

AH_InputSection::AH_InputSection() 
{
}

AH_InputSection::~AH_InputSection() {
  this->undefine();
}

void AH_InputSection::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}  

void AH_InputSection::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_InputSection::define()");
  undefine();

  int lowestFreeNode = 0;
  itsNSBF  = itsParamSet.getInt("NSBF");  // number of SubBand filters in the application
  
  
  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp;
  setComposite(comp); // tell the AppllicationHolder this is the top-level compisite

  // Create the InputSection using CEPFrame
  // The processing section consists of ...
  //todo: finish description

  LOG_TRACE_FLOW_STR("Create input side interface stubs");
  // RSP_Stub inStub(true);
  // todo: define this input interface; although there are no
  //       connection involved, we do have to define the port/IP numbering schemes

  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  Stub_FIR outStub(true);

  //todo: define simulated RSP boards here or in extra AH


  LOG_TRACE_FLOW_STR("Create the FringeCorrection workholder");
  //todo:define Fringe Step/WH
  LOG_TRACE_FLOW_STR("Create the Merge  workholder");
  //todo: define Merge Step/WH

  LOG_TRACE_FLOW_STR("Create the Synchronisation workholder");
//   Step*           SyncStep;
//   WH_SyncControl* SyncNode;
//   int nameSize = 40;
//   char name[nameSize];    
//   sprintf(name, "Sync_node_1_of_1");
//   SyncNode = new WH_SyncControl(name,
// 				itsParamSet.getInt("NRSP")); // one output connection per RSP
//   SyncStep = new Step(SyncNode, name,false);
//   itsWHs.push_back((WorkHolder*) SyncNode);
//   itsSteps.push_back(SyncStep);
//   SyncStep->runOnNode(lowestFreeNode++); //todo: define correct node number
//   comp.addBlock(SyncStep); // add to the top-level composite


  LOG_TRACE_FLOW_STR("Create the RSP reception Steps");
  // first determine the number of Transpose Steps that will be 
  // constructed later on; we need this number to define the output
  // DataHolders in the RSPInput Steps.
  // Note that the number of SubBandFilters per TRanspose Step
  // is hard codes as 2.
  const int NSBF = itsParamSet.getInt("NSBF");
  DBGASSERTSTR(NSBF%2 == 0, "NSBF should be an even number");
  const int NrTransposeNodes = NSBF/2;
  vector<Step*>        RSPSteps;
  vector<WH_RSPInput*> RSPNodes;
  int noRSPs = itsParamSet.getInt("NRSP");
  int WH_DH_NameSize = 40;
  char WH_DH_Name[WH_DH_NameSize];
  for (int r=0; r<noRSPs; r++) {
    snprintf(WH_DH_Name, WH_DH_NameSize, "RSP_Input_node_%d_of_%d", r, noRSPs);
    // todo: get interface and MACs from parameterset
    // todo: replace kvm by parameterSet
    if (r==0)
    {
      RSPNodes.push_back(new WH_RSPInput(WH_DH_Name,  // create sync master
					 itsParamSet,
					 "eth1",
					 "srcMac",
					 "dstMac",
					 true));
    }
    else
    {
      RSPNodes.push_back(new WH_RSPInput(WH_DH_Name,  // create slave
					 itsParamSet,
					 "eth1",
					 "srcMac",
					 "dstMac",
					 false));
    }
    RSPSteps.push_back(new Step(RSPNodes[r],WH_DH_Name,false));
    itsWHs.push_back((WorkHolder*) RSPNodes[r]);
    itsSteps.push_back(RSPSteps[r]);
    RSPSteps[r]->runOnNode(lowestFreeNode++);   
    comp.addBlock(RSPSteps[r]);

    // connect the RSP boards
    //todo: set correct IP/Port numbers in WH_RSP
    
    // Connect the SyncController
#ifdef HAVE_MPI
    SyncStep->connect(r, RSPSteps[r], 0, 1, 
		      new TH_MPI(SyncStep->getNode(), RSPSteps[r]->getNode()),
		      true);
#else
    LOG_WARN("No connection made to SyncController"); 
#endif

//     RSPSteps[r]->connect(itsRSPinputSteps[r], 
// 			    0, 0, 1,  //todo: check
// 			    TH_MPI(), 
// 			    true); //blocking
    
  };
  

  LOG_TRACE_FLOW_STR("Create the Transpose workholders");
  vector<WH_Transpose*> TransNodes;
  vector<Step*>          TransSteps;
  for (int r=0; r < NrTransposeNodes; r++) {
    sprintf(WH_DH_Name, "Transpose_node_%d_of_%d", r, noRSPs);
    TransNodes.push_back(new WH_Transpose(WH_DH_Name,      // name
					  KeyValueMap()));  // inputs  
    TransSteps.push_back(new Step(TransNodes[r],WH_DH_Name,false));
    itsWHs.push_back((WorkHolder*) TransNodes[r]);
    itsSteps.push_back(TransSteps[r]);
    TransSteps[r]->runOnNode(lowestFreeNode++);   
    comp.addBlock(TransSteps[r]);

    // connect the Subband filter form AH_BGLProcessing
    // to the input section
    // this interface is defined in the Stub_FIR class
    // Each Transpose node is connected to two SubBandfilters
    //
    // Output channel 0
    outStub.connect (2*r,                                                              // Corr filter number
		     TransNodes[r]->getDataManager(), 
		     0);  
    // Output channel 1
    outStub.connect (2*r+1,                                                              // Corr filter number
		     TransNodes[r]->getDataManager(),
		     1);  

  }      

  // todo: connect Transpose Steps to RSP Steps; 
  // this is a Transpose style connection that sends all
  // corresponding subbands to the same Transpose Step. 
  //  
  for (int t=0; t<NrTransposeNodes; t++) {
    for (int r=0; r<itsParamSet.getInt("NRSP"); r++) {
//       TransSteps[t]->connect(RSPSteps[r],
// 			    r,t,1,
// 			    TH_MPI(),
// 			    true);  // true=blocking
    }
  }
      

  LOG_TRACE_FLOW_STR("Finished define()");
}



void AH_InputSection::prerun() {
  getComposite().preprocess();
}
    
void AH_InputSection::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_InputSection::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_InputSection::run() "  );
}

void AH_InputSection::dump() const {
  LOG_TRACE_FLOW_STR("AH_InputSection::dump() not implemented"  );
}

void AH_InputSection::quit() {

}


