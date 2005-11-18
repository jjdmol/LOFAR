//#  AH_Recorder.cc: 
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

#include <AH_Recorder.h>

// Transporters
#include <Transport/TH_Mem.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Ethernet.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <WH_Signal.h>
#include <WH_FakeStation.h>
#include <WH_Strip.h>
#include <WH_Wrap.h>

using namespace LOFAR;

AH_Recorder::AH_Recorder() 
{
}

AH_Recorder::~AH_Recorder() 
{
  this->undefine();
}

void AH_Recorder::undefine() {
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
}  

void AH_Recorder::define(const LOFAR::KeyValueMap&) {
  LOG_TRACE_FLOW_STR("Start of AH_Recorder::define()");
  undefine();
    
  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

  int lastFreeNode = 0;
#ifdef HAVE_MPICH
  // mpich tries to run the first process on the local node
  // this is necessary if you want to use totalview
  // scampi run the first process on the first node in the machinefile
  lastFreeNode = 1;
#endif

  int NRSP = itsParamSet.getInt32("Input.NRSP");
  int WH_DH_NameSize = 40;
  char WH_DH_Name[WH_DH_NameSize];
  vector<string> outFileNames = itsParamSet.getStringVector("Generator.OutputFiles");
  vector<string> interfaces = itsParamSet.getStringVector("Generator.Interfaces");
  vector<string> dstMacs = itsParamSet.getStringVector("Input.DestinationMacs");
  vector<string> srcMacs = itsParamSet.getStringVector("Input.SourceMacs");
  int bufferSize = itsParamSet.getInt32("Generator.RecordBufferSize");
  
  snprintf(WH_DH_Name, WH_DH_NameSize, "Signal");

  for (int s=0; s<NRSP; s++) {
    //itsTHs.push_back(new TH_File("Generator1.in", TH_File::Read));
    cout<<"Creating TH_Ethernet: "<<srcMacs[s]<<" -> "<<dstMacs[s]<<endl;
    itsTHs.push_back(new TH_Ethernet(interfaces[s],
				     srcMacs[s],
				     dstMacs[s]));
    itsWHs.push_back(new WH_Wrap(WH_DH_Name,
				 *itsTHs.back(),
				 itsParamSet));
    Step* inStep = new Step(itsWHs.back(), WH_DH_Name);
    inStep->setOutBuffer(0, false, 100);
    itsSteps.push_back(inStep);
    comp.addBlock(inStep);
    inStep->runOnNode(lastFreeNode++);

    itsTHs.push_back(new TH_File(outFileNames[s], TH_File::Write));

    itsWHs.push_back(new WH_Strip(WH_DH_Name,
				  *itsTHs.back(),
				  itsParamSet));
    Step* outStep = new Step(itsWHs.back(), WH_DH_Name);
    itsSteps.push_back(outStep);
    comp.addBlock(outStep);
    outStep->runOnNode(lastFreeNode++);
    
#ifdef HAVE_MPI
    // this needs to be done with MPI, so if we don't have MPI do nothing
    outStep->connect(0, inStep, 0, 1,
		      new TH_MPI(inStep->getNode(),
				 outStep->getNode()),
		      true);
#else
    ASSERTSTR(false, "This application is supposed to be run with MPI");
#endif
  }

  // This program was written to run with MPI. All workholders run in their own process.
  // The machinefile should contain every node name twice, so WH_Wrap and WH_Strip run on 
  // the same physical host.
#ifdef HAVE_MPI
  ASSERTSTR (lastFreeNode == TH_MPI::getNumberOfNodes(), lastFreeNode << " nodes needed, "<<TH_MPI::getNumberOfNodes()<<" available");
#else
  ASSERTSTR(false, "This application is supposed to be run with MPI");
#endif

  LOG_TRACE_FLOW_STR("Finished defineRecorder()");
}



void AH_Recorder::prerun() {
  getComposite().preprocess();
}
    
void AH_Recorder::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_Recorder::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_Recorder::run() "  );
}

void AH_Recorder::dump() const {
  LOG_TRACE_FLOW_STR("AH_Recorder::dump() not implemented"  );
}

void AH_Recorder::quit() {
}


