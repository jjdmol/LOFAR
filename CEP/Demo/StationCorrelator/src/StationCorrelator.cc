//#  StationCorrelator.cc: Main program station correlator
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/KeyParser.h>
#include <Common/KeyValueMap.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>

#include <Transport/TH_Mem.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_Ethernet.h>

#include <tinyCEP/WorkHolder.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/ApplicationHolder.h>

#include <StationCorrelator.h>

#include <WH_RSPBoard.h>
#include <WH_RSP.h>
#include <WH_Transpose.h>
#include <WH_Correlator.h>
#include <WH_Dump.h>

#include <DH_RSP.h>
#include <DH_StationData.h>
#include <DH_CorrCube.h>
#include <DH_Vis.h>

using namespace LOFAR;

StationCorrelator::StationCorrelator(KeyValueMap kvm) 
  : itsKVM(kvm)
{
  itsNrsp = itsKVM.getInt("NoWH_RSP", 2);
  itsNcorrelator = itsKVM.getInt("NoWH_Correlator", 7);
  itsNdump = itsKVM.getInt("NoWH_Dump", 2);
}

StationCorrelator::~StationCorrelator() {
  this->undefine();
}

void StationCorrelator::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}  

void StationCorrelator::define(const KeyValueMap& /*kvm*/) {

  LOG_TRACE_FLOW_STR("Read KVM parameters");
  char H_name[128];
  int fast_rate = itsKVM.getInt("samples",256000)/itsKVM.getInt("NoPacketsInFrame",8);

  LOG_TRACE_FLOW_STR("Create the top-level composite");
  WH_Empty empty;
  Composite comp(empty);
  setComposite(comp);
  comp.runOnNode(0);

  LOG_TRACE_FLOW_STR("Create the workholders");
  /// Create the WorkHolders 
  bool useRealRSP = itsKVM.getBool("useRealRSPBoard", false);
  WH_RSPBoard RSPBoard("WH_RSPBoard", itsKVM);
  Step StepRSPemulator(RSPBoard, "STEP_RSPBoard");
  
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

    if (useRealRSP) {
      // The constructor for TH_Ethernet also expects etype and dhcheck
      // There is no error checking. If this fails the program will fail (and that is fine)
      string iface = itsKVM["interfaces"].getVecString()[i];
      cout<<"interface: "<<iface<<endl;
      string oMac  = itsKVM["oMacs"].getVecString()[i];
      string rMac  = itsKVM["rMacs"].getVecString()[i];
      // The constructor for TH_Ethernet expects char* instead of const char* or string
      // That should change. It also should copy the contents of the char*s to its own memory
      //itsRSPsteps[i]->connect(&StepRSPemulator, 0, i, 1, TH_Ethernet(iface.c_str(), rMac.c_str(), oMac.c_str()), true);
      StepRSPemulator.runOnNode(-1);
    } else {
      // Use the WH_RSPBoard to emulate a real RSP Board
      itsRSPsteps[i]->connect(&StepRSPemulator, 0, i, 1, TH_Mem(), true); 
    }

    // set the rates of this Step.
    itsRSPsteps[i]->setInRate(fast_rate);
    itsRSPsteps[i]->setProcessRate(fast_rate);
    itsRSPsteps[i]->setOutRate(fast_rate);

    comp.addStep(itsRSPsteps[i]); 

    if (i != 0) {
      // we're a syncSlave. Connect the second input to an appropriate output.
      itsRSPsteps[i]->connect(itsRSPsteps[0], 1, itsNcorrelator + i - 1, 1, TH_Mem(), true);
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
    Step dumpstep(whDump, H_name);
    comp.addStep(dumpstep);
    
    for (unsigned int in = 0; in < (itsNcorrelator/itsNdump); in++) {
      dumpstep.connect(itsCsteps[c_index++], in, 0, 1, TH_Mem(), 
		       true);  // true=blocking
    }
  }
  LOG_TRACE_FLOW_STR("Finished define()");
}

void StationCorrelator::prerun() {
  getComposite().preprocess();
}
    
void StationCorrelator::run(int steps) {
  LOG_TRACE_FLOW_STR("Start StationCorrelator::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished StationCorrelator::run() "  );
}

void StationCorrelator::dump() const {
  LOG_TRACE_FLOW_STR("StationCorrelator::dump() not implemented"  );
}

void StationCorrelator::quit() {

}

int main (int argc, const char** argv) {

  INIT_LOGGER("StationCorrelator");

  try {
    kvm = KeyParser::parseFile("TestRange");

  } catch (std::exception& x) {
    cerr << x.what() << endl;
  }
  
  try {
    //    kvm.show(cout);

    StationCorrelator correlator(kvm);
    correlator.setarg(argc, argv);
    correlator.baseDefine(kvm);
    cout << "defined" << endl;
    correlator.basePrerun();
    cout << "init done" << endl;
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
