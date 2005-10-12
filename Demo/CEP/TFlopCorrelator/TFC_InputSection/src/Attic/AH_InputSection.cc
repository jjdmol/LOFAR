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
#include <Transport/TH_File.h>
#include <Transport/TH_Mem.h>
#include <Transport/TH_Ethernet.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <TFC_InputSection/WH_RSPInput.h>
#include <TFC_InputSection/WH_SBCollect.h>
#include <TFC_Interface/Stub_FIR.h>
#include <TFC_Interface/Stub_Delay.h>

using namespace LOFAR;

AH_InputSection::AH_InputSection() 
  : itsNSBF       (0),
    itsInputStub  (0),
    itsOutputStub (0)
{
}

AH_InputSection::~AH_InputSection() {
  this->undefine();
}

void AH_InputSection::undefine() {
  
  vector<WorkHolder*>::iterator wit = itsWHs.begin();
  for (; wit!=itsWHs.end(); wit++) {
    delete *wit;
  }
  itsWHs.clear();
  
  vector<Step*>::iterator sit = itsSteps.begin();
  for (; sit!=itsSteps.end(); sit++) {
    delete *sit;
  }
  itsSteps.clear();
  
  vector<TransportHolder*>::iterator tit = itsTHs.begin();
  for (; tit!=itsTHs.end(); tit++) {
    delete *tit;
  }
  itsTHs.clear();

  delete itsInputStub;
  delete itsOutputStub;
  itsInputStub = 0;
  itsOutputStub = 0;
}  

void AH_InputSection::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_InputSection::define()");
  undefine();

  int lowestFreeNode = 0;
  itsNSBF  = itsParamSet.getInt32("Input.NSubbands");  // number of SubBand filters in the application
    
  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

  LOG_TRACE_FLOW_STR("Create the input side delay stub");
  itsInputStub = new Stub_Delay(true, itsParamSet);

  LOG_TRACE_FLOW_STR("Create the RSP reception Steps");
  /* first determine the number of Transpose Steps that will be 
     constructed later on; we need this number to define the output
     DataHolders in the RSPInput Steps.
     Note that the number of SubBandFilters per Transpose Step
     is hard coded as 2 */
//   DBGASSERTSTR(itsNSBF%2 == 0, "NSBF should be an even number");
//   const int NrTransposeNodes = itsNSBF/2;

  // Right now there is 1 Transpose node per subbandFilter
  
  vector<Step*>        RSPSteps;
  vector<WH_RSPInput*> RSPNodes;
  int NRSP = itsParamSet.getInt32("Input.NRSP");
  int WH_DH_NameSize = 40;
  char WH_DH_Name[WH_DH_NameSize];
  int rspStartNode;
  
  bool useEth = itsParamSet.getBool("Input.UseEth");
  vector<string> interfaces = itsParamSet.getStringVector("Input.Interfaces");
  vector<string> srcMacs = itsParamSet.getStringVector("Input.SourceMacs");
  vector<string> dstMacs = itsParamSet.getStringVector("Input.DestinationMacs");
  vector<string> inFiles = itsParamSet.getStringVector("Input.InputFiles");
    
  for (int r=0; r<NRSP; r++) {
    snprintf(WH_DH_Name, WH_DH_NameSize, "RSP_Input_node_%d_of_%d", r, NRSP);
    
    if (useEth) {
      itsTHs.push_back(new TH_Ethernet(interfaces[r], 
				       srcMacs[r],
				       dstMacs[r], 
				       0x000));
    } else {
      itsTHs.push_back(new TH_File(inFiles[r], TH_File::Read));
    }

    if (r==0)
    {
      RSPNodes.push_back(new WH_RSPInput(WH_DH_Name,  // create sync master
					 itsParamSet,
                                         *itsTHs.back(),
					 true));
      rspStartNode = lowestFreeNode;
    }
    else
    {
       RSPNodes.push_back(new WH_RSPInput(WH_DH_Name,  // create slave
	 				  itsParamSet,
                                          *itsTHs.back(),
					  false));
    }
    RSPSteps.push_back(new Step(RSPNodes[r],WH_DH_Name,false));
    itsWHs.push_back((WorkHolder*) RSPNodes[r]);
    itsSteps.push_back(RSPSteps[r]);
    RSPSteps[r]->runOnNode(lowestFreeNode++);   
    comp.addBlock(RSPSteps[r]);
    
    // Connect the Delay Controller
    itsInputStub->connect(r, (RSPSteps.back())->getInDataManager(0), 0);
    if (r!=0) {
      int sourceStep = itsSteps.size() - r - 1;
#ifdef HAVE_MPI
      itsSteps.back()->connect(1,  // input number 0 is the delay, 1 is the sync
			       itsSteps[sourceStep],
			       itsNSBF + r - 1,   // there are NSubbands outputs
			       1, // connect 1 DH
			       new TH_MPI(itsSteps[sourceStep]->getNode(), (itsSteps.back())->getNode()),
			       true);
#else
      itsSteps.back()->connect(1,  // input number 0 is the delay
			       itsSteps[sourceStep],
			       itsNSBF + r - 1,
			       1,
			       new TH_Mem(),
			       false);
#endif
    }    
  };
  
  LOG_TRACE_FLOW_STR("Create output side interface stubs");
  itsOutputStub = new Stub_FIR(true, itsParamSet);

  LOG_TRACE_FLOW_STR("Create the Subband merger workholders");
  vector<WH_SBCollect*> collectNodes;
  vector<Step*>         collectSteps;
  int collectStartNode;
  for (int nf=0; nf < itsNSBF; nf++) {
    sprintf(WH_DH_Name, "Collect_node_%d_of_%d", nf, NRSP);
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

    // Connect splitters to mergers (transpose)
    for (int st=0; st<NRSP; st++)
    {
#ifdef HAVE_MPI
      collectSteps[nf]->connect(st, RSPSteps[st], nf, 1,
			       new TH_MPI(rspStartNode+st, collectStartNode+nf), 
			       true);
#else
      collectSteps[nf]->connect(st, RSPSteps[st], nf, 1, new TH_Mem(), false);
#endif
    }
    // connect output to FIR stub
    itsOutputStub->connect (nf,                           // Corr filter number
			    (collectSteps.back())->getOutDataManager(0), 
			    0);  
  }
  LOG_TRACE_FLOW_STR("Finished define()");

#ifdef HAVE_MPI
  ASSERTSTR (lowestFreeNode == TH_MPI::getNumberOfNodes(), "TFC_InputSection needs "<< lowestFreeNode << " nodes, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif
}



void AH_InputSection::prerun() {
  getComposite().preprocess();
}
    
void AH_InputSection::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_InputSection::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    cout<<"run "<<i+1<<" of "<<steps<<endl;
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_InputSection::run() "  );
}

void AH_InputSection::dump() const {
  LOG_TRACE_FLOW_STR("AH_InputSection::dump() not implemented"  );
}

void AH_InputSection::quit() {

}

