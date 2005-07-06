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

#include <APS/ParameterSet.h>

#include <AH_InputSection.h>

// Transporters
#include <Transport/TH_MPI.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <TFC_InputSection/WH_RSPInput.h>
#include <TFC_InputSection/WH_SBSplit.h>
#include <TFC_InputSection/WH_SBCollect.h>
#include <TFC_Interface/Stub_FIR.h>
#include <TFC_Interface/Stub_Delay.h>

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
  Composite comp(0, 0, "topComposite");
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

  LOG_TRACE_FLOW_STR("Create the input side delay stub");
  Stub_Delay delayStub(true);

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
    
    // Connect the Delay Controller
    delayStub.connect(r, RSPNodes[r]->getDataManager(), 0);
    
  };
  

  LOG_TRACE_FLOW_STR("Create the Subband splitter workholders");
  vector<WH_SBSplit*> splitNodes;
  vector<Step*>       splitSteps;
  int splitStartNode;
  for (int r=0; r < noRSPs; r++) {
    sprintf(WH_DH_Name, "Split_node_%d_of_%d", r, noRSPs);
    splitNodes.push_back(new WH_SBSplit(WH_DH_Name,      // name
					itsParamSet)); // inputs  
    splitSteps.push_back(new Step(splitNodes[r],WH_DH_Name,false));
    itsWHs.push_back((WorkHolder*) splitNodes[r]);
    itsSteps.push_back(splitSteps[r]);
    if (r==0)
    {
      splitStartNode = lowestFreeNode;
    }
    splitSteps[r]->runOnNode(lowestFreeNode++);   
    comp.addBlock(splitSteps[r]);

#ifdef HAVE_MPI
    // Connect RSP steps to splitters
    splitSteps[r]->connect(0, RSPSteps[r], 0, 1, 
			   new TH_MPI(splitStartNode-noRSPs+r, splitStartNode+r),
			   true);
#endif
  }

  LOG_TRACE_FLOW_STR("Create the Subband merger workholders");
  vector<WH_SBCollect*> collectNodes;
  vector<Step*>         collectSteps;
  int collectStartNode;
  for (int nf=0; nf < NSBF; nf++) {
    sprintf(WH_DH_Name, "Split_node_%d_of_%d", nf, noRSPs);
    collectNodes.push_back(new WH_SBCollect(WH_DH_Name,      // name
					    nf,              // Subband ID
 					    itsParamSet));   // inputs  
    collectSteps.push_back(new Step(collectNodes[nf],WH_DH_Name,false));
    itsWHs.push_back((WorkHolder*) collectNodes[nf]);
    itsSteps.push_back(collectSteps[nf]);
    if (nf==0)
    {
      collectStartNode = lowestFreeNode;
    }
    collectSteps[nf]->runOnNode(lowestFreeNode++);   
    comp.addBlock(collectSteps[nf]);

#ifdef HAVE_MPI
    // Connect splitters to mergers (transpose)
    for (int st=0; st<noRSPs; st++)
    {
      collectSteps[nf]->connect(st, splitSteps[st], nf, 1,
			       new TH_MPI(splitStartNode+st, collectStartNode+nf), 
			       true);
    }
#endif
    // connect output to FIR stub
    // Output channel 0
    outStub.connect (nf,                           // Corr filter number
		     collectNodes[nf]->getDataManager(), 
		     0);  
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


