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

#include <CEPFrame/Step.h>
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

void StationCorrelator::define(const KeyValueMap& itsKVM) {

  char H_name[128];

  vector<WH_RSP*>        RSPNodes;
  vector<WH_Transpose*>  TransposeNodes;
  vector<WH_Correlator*> CorrelatorNodes;
  vector<WH_Dump*>       DumpNodes;   
  
  Composite comp;
  setComposite(comp);

  /// Create the WorkHolders 
  for (unsigned int i = 0; i < itsNrsp; i++) {
    snprintf(H_name, 128, "RSPNode_%d_of_%d", i, itsNrsp);
    Step step(new WH_RSP(H_name));
    comp.addStep(step); // gaat dit goed?
  }

  for (unsigned int i = 0; i < itsNcorrelator; i++) {
    // we create a transpose workholder for every correlator workholder
    // to rearrange the data coming from the RSP boards.
    sprintf(H_name, "TransposeNode_%d_of_%d", i, itsNcorrelator);

    Step step(new WH_Transpose(H_name, itsKVM));
    comp.addStep(step);
  }
  for (unsigned int i = 0; i < itsNcorrelator; i++) {

    sprintf(H_name, "CorrelatorNode_%d_of_%d", i, itsNcorrelator);
    
    Step step(new WH_Correlator ( H_name,
				   0,
				   0,
				   0,
				   0,
				   0 ));
    comp.addStep(step);
  }

  for (unsigned int i = 0; i < itsNdump; i++) {
    sprintf(H_name, "DumpNode_%d_of_%d", i, itsNdump);
    
    Step step(new WH_Dump (H_name, 
			   0,
			   0, 
			   0));
    comp.addStep(step);
  }

}

void StationCorrelator::init() {
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

  KeyValueMap kvm;
  try {
    kvm = KeyParser::parseFile("TestRange");

  } catch (std::exception& x) {
    cerr << x.what() << endl;
  }
  
  try {

    StationCorrelator correlator(kvm);
    correlator.setarg(argc, argv);
    correlator.baseDefine();
    cout << "defined"<< endl;
    correlator.basePrerun();
    cout << "initialized" << endl;
    correlator.baseRun();
    cout << "finished" << endl;
    correlator.baseDump();
    correlator.baseQuit();

  } catch (...) {
    cout << "Unexpected exception" << endl;
  }
  return 0;
}
