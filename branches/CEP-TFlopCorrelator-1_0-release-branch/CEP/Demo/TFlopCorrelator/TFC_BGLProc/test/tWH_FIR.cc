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
#include <tWH_FIR.h>

#include <Transport/TH_Mem.h>
#include <TFC_BGLProc/WH_FIR.h>
#include <TFC_Interface/DH_FIR.h>
#include <TFC_Interface/DH_PPF.h>

namespace LOFAR
{

  AH_FIR::AH_FIR() :
    itsWH(0), itsInDH1(0), itsOutDH1(0), itsOutDH2(0), itsInCon1(0), 
    itsOutCon1(0), itsOutCon2(0)
  {}

  AH_FIR::~AH_FIR() {
  }
  
  void AH_FIR::define(const KeyValueMap& kvm) {
    KeyValueMap myKvm(kvm);

    ACC::APS::ParameterSet myPset("TFlopCorrelator.cfg");

    itsInDH1 = new DH_FIR("itsInDH1", 0, myPset);
    
    itsOutDH1 = new DH_PPF("itsOutDH1", 0);
    itsOutDH2 = new DH_PPF("itsOutDH2", 0);

    itsWH = new WH_FIR("WH_FIR", 0);
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

    itsOutCon1 = new Connection("out2", 
				itsWH->getDataManager().getOutHolder(1), 
				itsOutDH2, 
				itsTH, 
				false);
  }

  void AH_FIR::init() {
    itsWH->basePreprocess();
  }

  void AH_FIR::run(int steps) {
    for (int i = 0; i<steps; i++) {
      itsWH->baseProcess();
    }
  }

  void AH_FIR::postrun() {
    // check result here
    
  }

  void AH_FIR::undefine() {
    delete itsWH;

    delete itsInDH1; 

    delete itsOutDH1;
    delete itsOutDH2;

    delete itsInCon1;

    delete itsOutCon1;
    delete itsOutCon2;
  }

  void AH_FIR::quit() {
  }

} // namespace LOFAR


using namespace LOFAR;

int main (int argc, const char** argv) {
  try {
    AH_FIR test;
    test.setarg(argc, argv);
    test.baseDefine();
    test.basePrerun();
    test.baseRun(10);
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
