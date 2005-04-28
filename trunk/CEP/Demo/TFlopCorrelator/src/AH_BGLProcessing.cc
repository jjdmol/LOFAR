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
  WH_Empty empty;
  Composite comp(empty);
  setComposite(comp);
  comp.runOnNode(0);


  // Create the input section; these use  tinyCEP

  LOG_TRACE_FLOW_STR("Create the workholders");
  /// Create the WorkHolders 
  Step** itsRSPsteps = new Step*[itsNrsp];
  for (unsigned int i = 0; i < itsNrsp; i++) {
    snprintf(H_name, 128, "RSPNode_%d_of_%d", i, itsNrsp);
    LOG_TRACE_LOOP_STR("Create RSP workholder/Step " << H_name);
    WH_RSP* whRSP; 

    if (i == 0) {
      whRSP = new WH_RSP(H_name, itsKVM, true);  // syncmaster
    } else {
      whRSP = new WH_RSP(H_name, itsKVM, false);  // notsyncmaster
    }
    itsRSPsteps[i] = new Step(*whRSP, H_name, false);

    // set the rates of this Step.
    itsRSPsteps[i]->setInRate(fast_rate);
    itsRSPsteps[i]->setProcessRate(fast_rate);
    itsRSPsteps[i]->setOutRate(fast_rate);

    comp.addStep(itsRSPsteps[i]); 

    if (i != 0) {
      // we're a syncSlave. Connect the second input to an appropriate output.
      itsRSPsteps[i]->connect(itsRSPsteps[0], 1, itsNcorrelator, 1, TH_Mem(), true);
    }
  }

  Step** itsTsteps = new Step*[itsNcorrelator];
  Step** itsCsteps = new Step*[itsNcorrelator];
  for (unsigned int i = 0; i < itsNcorrelator; i++) {
    // we create a transpose workholder for every correlator workholder
    // to rearrange the data coming from the RSP boards.
    sprintf(H_name, "TransposeNode_%d_of_%d", i, itsNcorrelator);
    LOG_TRACE_LOOP_STR("Create Transpose workholder/Step " << H_name);

    WH_Transpose whTranspose(H_name, itsKVM);
    itsTsteps[i] = new Step(whTranspose, H_name, false);
    // the transpose collects data to intergrate over, so only the input 
    // and process methods run fast
    itsTsteps[i]->setInRate(fast_rate);
    itsTsteps[i]->setProcessRate(fast_rate);

    comp.addStep(itsTsteps[i]);

    // connect the Transpose step just created to the correct RSP outputs
    for (unsigned int rsp = 0; rsp < itsNrsp; rsp++) {
      //      itsRSPsteps[rsp]->connect(itsTsteps[i], i+1, rsp, 1, TH_Mem(), true);
      itsTsteps[i]->connect(itsRSPsteps[rsp], rsp, i, 1, TH_Mem(), 
			    true);       // true=blocking
    }


    // now create the Correlator workholder
    sprintf(H_name, "CorrelatorNode_%d_of_%d", i, itsNcorrelator);
    LOG_TRACE_LOOP_STR("Create Correlator  workholder/Step " << H_name);

    WH_Correlator whCorrelator(H_name, itsKVM);
    itsCsteps[i] = new Step(whCorrelator, H_name, false);
    comp.addStep(itsCsteps[i]);
    
    itsCsteps[i]->connectInput(itsTsteps[i], TH_Mem(), 
			       true);   // true=blocking
  }
  
  
  unsigned int c_index = 0;
  for (unsigned int i = 0; i < itsNdump; i++) {
    sprintf(H_name, "DumpNode_%d_of_%d", i, itsNdump);
    LOG_TRACE_LOOP_STR("Create Dump workholder/Step " << H_name);

    WH_Dump whDump(H_name, itsKVM);
    Step dumpstep(whDump);
    comp.addStep(dumpstep);
    
    for (unsigned int in = 0; in < (itsNcorrelator/itsNdump); in++) {
      dumpstep.connect(itsCsteps[c_index++], in, 0, 1, TH_Mem(), 
		       true);  // true=blocking
    }
  }
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

int main (int argc, const char** argv) {

  INIT_LOGGER("AH_BGLProcessing");

  try {
    kvm = KeyParser::parseFile("TestRange");

  } catch (std::exception& x) {
    cerr << x.what() << endl;
  }
  
  try {
//     kvm.show(cout);

    AH_BGLProcessing correlator(kvm);
    correlator.setarg(argc, argv);
    correlator.baseDefine(kvm);
    cout << "defined" << endl;
    correlator.basePrerun();
    cout << "init" << endl;
    correlator.baseRun(1);
    cout << "run" << endl;
    correlator.baseDump();
    correlator.baseQuit();

  } catch (std::exception& x) {
    cout << "Unexpected exception" << endl;
    cerr << x.what() << endl; 
  }
  return 0;
}
