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
#include <TH_RSP.h>

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
  int slow_rate = itsKVM.getInt("samples",256000)/itsKVM.getInt("NoPacketsInFrame",8);
  int lastFreeNode = 0;
  cout<<"slow_rate: "<<slow_rate<<endl;

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
  comp.addStep(StepRSPemulator);
  if (useRealRSP) {
    StepRSPemulator.runOnNode(-1);
  } else {
    StepRSPemulator.runOnNode(lastFreeNode++);
  }
  
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
    comp.addStep(itsRSPsteps[i]); 

    itsRSPsteps[i]->runOnNode(lastFreeNode++);

    if (useRealRSP) {
      string iface = itsKVM["interfaces"].getVecString()[i];
      cout<<"interface: "<<iface<<endl;
      string oMac  = itsKVM["oMacs"].getVecString()[i];
      string rMac  = itsKVM["rMacs"].getVecString()[i];
      itsRSPsteps[i]->connect(&StepRSPemulator, 0, i, 1, TH_RSP(iface.c_str(), 
								rMac.c_str(), 
								oMac.c_str(), 
								0x000, 
								true), true);
    } else {
      // Use the WH_RSPBoard to emulate a real RSP Board
      connect(&StepRSPemulator, itsRSPsteps[i], i, 0);
    }

    if (i != 0) {
      // we're a syncSlave. Connect the second input to an appropriate output.
      connect(itsRSPsteps[0], itsRSPsteps[i], itsNcorrelator + i - 1, 1);
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
    comp.addStep(itsTsteps[i]);
    // the transpose collects data to intergrate over, so only the input 
    // and process methods run fast
    itsTsteps[i]->setOutRate(slow_rate);
    itsTsteps[i]->runOnNode(lastFreeNode++); // do not increase lastFreeNode here, it needs to run on the same node

    // connect the Transpose step just created to the correct RSP outputs
    for (unsigned int rsp = 0; rsp < itsNrsp; rsp++) {
      connect(itsRSPsteps[rsp], itsTsteps[i], i, rsp);
    }


    // now create the Correlator workholder
    sprintf(H_name, "CorrelatorNode_%d_of_%d", i, itsNcorrelator);
    LOG_TRACE_LOOP_STR("Create Correlator  workholder/Step " << H_name);

    WH_Correlator whCorrelator(H_name, itsKVM);
    itsCsteps[i] = new Step(whCorrelator, H_name, false);
    comp.addStep(itsCsteps[i]);
    itsCsteps[i]->setInRate(slow_rate);
    itsCsteps[i]->setProcessRate(slow_rate);
    itsCsteps[i]->setOutRate(slow_rate);
    itsCsteps[i]->runOnNode(lastFreeNode++);
    
    connect(itsTsteps[i], itsCsteps[i], 0, 0);
  }
  
  
  unsigned int c_index = 0;
  for (unsigned int i = 0; i < itsNdump; i++) {
    sprintf(H_name, "DumpNode_%d_of_%d", i, itsNdump);
    LOG_TRACE_LOOP_STR("Create Dump workholder/Step " << H_name);

    string oFile  = itsKVM["outFileName"].getVecString()[i];

    WH_Dump whDump(H_name, itsKVM, oFile);
    Step dumpstep(whDump, H_name);
    comp.addStep(dumpstep);
    dumpstep.setInRate(slow_rate);
    dumpstep.setProcessRate(slow_rate);
    dumpstep.setOutRate(slow_rate);
    dumpstep.runOnNode(lastFreeNode++);
    
    for (unsigned int in = 0; in < (itsNcorrelator/itsNdump); in++) {
      connect(itsCsteps[c_index++], &dumpstep, 0, in);
    }
  }
  LOG_TRACE_FLOW_STR("Finished define()");

#ifdef HAVE_MPI
  ASSERTSTR (lastFreeNode == TH_MPI::getNumberOfNodes(), lastFreeNode << " nodes needed, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif
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
  LOG_TRACE_FLOW_STR("Start StationCorrelator::dump() ");
  getComposite().dump();
  LOG_TRACE_FLOW_STR("Finished StationCorrelator::dump() ");
}

void StationCorrelator::quit() {

}

void StationCorrelator::connect(Step* srcStep, Step* dstStep, int srcDH, int dstDH) {
  //  cout<<"Connecting "<<srcStep->getName()<<" and "<<dstStep->getName()<<" ...";
#ifdef HAVE_MPI
  int srcNode = srcStep->getNode();
  int dstNode = dstStep->getNode();
  //  cout<<" from "<<srcNode<<" to "<<dstNode<<" ";
  if (srcNode == dstNode) {
    //    cout<<srcStep->getName()<<" and "<<dstStep->getName()<<" on same node"<<endl;
    dstStep->connect(srcStep, dstDH, srcDH, 1, TH_Mem(), false);  // true=blocking
  } else {
    dstStep->connect(srcStep, dstDH, srcDH, 1, TH_MPI(srcNode, dstNode), true);  // true=blocking
  }
#else
  dstStep->connect(srcStep, dstDH, srcDH, 1, TH_Mem(), false);  // true=blocking
#endif
  //  cout<<"ok"<<endl;
}

int main (int argc, const char** argv) {

  INIT_LOGGER("StationCorrelator");

  try {
    kvm = KeyParser::parseFile("/home/zwart/TestRange");

    StationCorrelator correlator(kvm);
    correlator.setarg(argc, argv);
    correlator.baseDefine(kvm);
    cout << "defined" << endl;
    correlator.basePrerun();
    cout << "init done" << endl;
    correlator.baseRun(kvm.getInt("runsteps",1));
    cout << "run" << endl;
    correlator.baseDump();
    correlator.baseQuit();

  } catch (std::exception& x) {
    cout << "Unexpected exception" << endl;
    cerr << x.what() << endl; 
  }
  return 0;
}
