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
#include <ACC/ParameterSet.h>
#include <tWH_Correlator.h>

#include <Transport/TH_Mem.h>
#include <TFC_BGLProc/WH_Correlator.h>
#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>

namespace LOFAR
{

  AH_Correlator::AH_Correlator() :
    itsWH(0),
    itsInDH1(0), itsInDH2(0), itsOutDH1(0), itsOutDH2(0), itsOutDH3(0),
    itsOutDH4(0), itsOutDH5(0), itsInCon1(0), itsInCon2(0), itsOutCon1(0),
    itsOutCon2(0), itsOutCon3(0), itsOutCon4(0), itsOutCon5(0)
  {}

  AH_Correlator::~AH_Correlator() {
  }
  
  void AH_Correlator::define(const KeyValueMap& kvm) {
    KeyValueMap myKvm(kvm);

    itsInDH1 = new DH_PPF("itsInDH1", 0);
    itsInDH2 = new DH_PPF("itsInDH2", 0);
    
    itsOutDH1 = new DH_CorrCube("itsOutDH1", 0);
    itsOutDH2 = new DH_CorrCube("itsOutDH2", 0);
    itsOutDH3 = new DH_CorrCube("itsOutDH3", 0);
    itsOutDH4 = new DH_CorrCube("itsOutDH4", 0);
    itsOutDH5 = new DH_CorrCube("itsOutDH5", 0);

    itsWH = new WH_Correlator("WH_Correlator");
    itsTH = new TH_Mem();

    itsInCon1 = new Connection("in1", 
			       itsInDH1, 
			       itsWH->getDataManager().getInHolder(0), 
			       itsTH, 
			       false);

    itsInCon2 = new Connection("in2", 
			       itsInDH2, 
			       itsWH->getDataManager().getInHolder(1), 
			       itsTH,
			       false);

    itsOutCon1 = new Connection("out1", 
				itsOutDH1, 
				itsWH->getDataManager().getOutHolder(0), 
				itsTH, 
				false);

    itsOutCon2 = new Connection("out2", 
				itsOutDH2, 
				itsWH->getDataManager().getOutHolder(1), 
				itsTH, 
				false);
    itsOutCon3 = new Connection("out3", 
				itsOutDH3, 
				itsWH->getDataManager().getOutHolder(2), 
				itsTH, 
				false);
    itsOutCon4 = new Connection("out4", 
				itsOutDH4, 
				itsWH->getDataManager().getOutHolder(3), 
				itsTH, 
				false);
    itsOutCon5 = new Connection("out5", 
				itsOutDH5, 
				itsWH->getDataManager().getOutHolder(4), 
				itsTH, 
				false);
  }

  void AH_Correlator::init() {
    itsWH->basePreprocess();
  }

  void AH_Correlator::run(int steps) {
    for (int i = 0; i<steps; i++) {
      itsWH->baseProcess();
    }
  }

  void AH_Correlator::postrun() {
    // check result here
    
  }

  void AH_Correlator::undefine() {
    delete itsWH;

    delete itsInDH1; 
    delete itsInDH2; 

    delete itsOutDH1;
    delete itsOutDH2;
    delete itsOutDH3;
    delete itsOutDH4;
    delete itsOutDH5;
    
    delete itsInCon1;
    delete itsInCon2;

    delete itsOutCon1;
    delete itsOutCon2;
    delete itsOutCon3;
    delete itsOutCon4;
    delete itsOutCon5;
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
