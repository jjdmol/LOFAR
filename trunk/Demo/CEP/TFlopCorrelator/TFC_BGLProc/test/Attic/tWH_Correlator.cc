//#  filename.cc: one line description
//#
//#  Copyright (C) 2002-2005
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

#include <Common/LofarLogger.h>
#include <APS/ParameterSet.h>
#include <tWH_Correlator.h>

#include <Transport/TH_Mem.h>
#include <TFC_BGLProc/WH_Correlator.h>
#include <TFC_Interface/DH_FIR.h>
// #include <TFC_Interface/DH_CorrCube.h>
#include <TFC_Interface/DH_Vis.h>

namespace LOFAR
{

  AH_Correlator::AH_Correlator() :
    itsWH(0),
    itsInDH1(0), itsOutDH1(0), itsInCon1(0), itsOutCon1(0)
  {}

  AH_Correlator::~AH_Correlator() {
    
    this->undefine();

  }
  
  void AH_Correlator::define(const KeyValueMap& /*kvm*/) {

    ACC::APS::ParameterSet myPset("TFlopCorrelator.cfg");

    itsInDH1 = new DH_FIR("itsIn1",0, myPset);
//     itsInDH1 = new DH_CorrCube("itsIn1",0);
    itsOutDH1 = new DH_Vis("itsOutDH1", 0, myPset);

    itsWH = new WH_Correlator("WH_Correlator");
    itsTH = new TH_Mem();

    itsInCon1 = new Connection("in1", 
			       itsInDH1, 
			       itsWH->getDataManager().getInHolder(0), 
			       itsTH, 
			       false);

    itsOutCon1 = new Connection("out1", 
				itsWH->getDataManager().getOutHolder(0), 
				itsOutDH1, 
				itsTH, 
				false);

    
  }

  void AH_Correlator::init() {
    itsWH->basePreprocess();

    // Fill inDHs here
    static_cast<DH_FIR*>(itsWH->getDataManager().getInHolder(0))->setCorrelatorTestPattern();
//     static_cast<DH_CorrCube*>(itsWH->getDataManager().getInHolder(0))->setTestPattern();
  }

  void AH_Correlator::run(int steps) {
    for (int i = 0; i<steps; i++) {
      itsWH->baseProcess();
    }
  }

  void AH_Correlator::dump() const {
    itsWH->dump();
  }

  void AH_Correlator::postrun() {
    // check result here
    cout << "Result = " << 
      (bool) static_cast<DH_Vis*>(itsWH->getDataManager().getOutHolder(0))->checkCorrelatorTestPattern()
    << endl;
  }

  void AH_Correlator::undefine() {
    delete itsWH;

    delete itsInDH1; 
    delete itsOutDH1;
    
    delete itsInCon1;
    delete itsOutCon1;

    delete itsTH;
  }

  void AH_Correlator::quit() {
  }

} // namespace LOFAR


using namespace LOFAR;

int main (int argc, const char** argv) {
  try {
    AH_Correlator test;
    test.setarg(argc, argv);
    test.baseDefine();
    test.basePrerun();
    test.baseRun(1);
    test.baseDump();
    test.basePostrun();
    test.baseQuit();

  } catch (LOFAR::Exception e) {
    cerr << "Caught exception: " << e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception " << endl;
    exit(1);
  }
  return 0;
}
