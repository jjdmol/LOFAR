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

#include <tinyCEP/WorkHolder.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/ApplicationHolder.h>

#include <StationCorrelator.h>

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
  itsNrsp = itsKVM.getInt("rsps", 2);
  itsNcorrelator = itsKVM.getInt("correlators", 7);
  itsNdump = itsKVM.getInt("dumps", 1);
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

  char H_name[128];
  char old_name[128];

  vector<WH_RSP*>        RSPNodes;
  vector<WH_Transpose*>  TransposeNodes;
  vector<WH_Correlator*> CorrelatorNodes;
  vector<WH_Dump*>       DumpNodes;   
  
  WH_Empty empty;
  Composite comp(empty);
  setComposite(comp);
  comp.runOnNode(0);

  /// Create the WorkHolders 
  for (unsigned int i = 0; i < itsNrsp; i++) {
    snprintf(H_name, 128, "RSPNode_%d_of_%d", i, itsNrsp);
    
    WH_RSP whRSP(H_name, itsKVM);
    Step step(whRSP, H_name, false);
    comp.addStep(step); 
  }

  for (unsigned int i = 0; i < itsNcorrelator; i++) {
    // we create a transpose workholder for every correlator workholder
    // to rearrange the data coming from the RSP boards.
    sprintf(H_name, "TransposeNode_%d_of_%d", i, itsNcorrelator);

    WH_Transpose whTranspose(H_name, itsKVM);
    Step step1(whTranspose);
    comp.addStep(step1);

//     step1.connect("");

    sprintf(H_name, "CorrelatorNode_%d_of_%d", i, itsNcorrelator);

    WH_Correlator whCorrelator(H_name, itsKVM);
    Step step2(whCorrelator);
    comp.addStep(step2);

    step2.connectInput(&step1, TH_Mem());
  }

  for (unsigned int i = 0; i < itsNdump; i++) {
    sprintf(H_name, "DumpNode_%d_of_%d", i, itsNdump);

    WH_Dump whDump(H_name, itsKVM);
    Step step(whDump);
    comp.addStep(step);

//    step.connectInput();
  }
}

void StationCorrelator::prerun() {
  getComposite().preprocess();
}
    
void StationCorrelator::run(int steps) {
  for (int i = 0; i < steps; i++) {
    getComposite().process();
  }
}

void StationCorrelator::dump() const {

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
//     kvm.show(cout);

    StationCorrelator correlator(kvm);
    correlator.setarg(argc, argv);
    correlator.baseDefine(kvm);
    cout << "defined" << endl;
    correlator.basePrerun();
    cout << "init" << endl;
    correlator.baseRun(1);
    cout << "run" << endl;
    correlator.baseDump();
    correlator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
  return 0;
}
