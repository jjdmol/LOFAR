//#  tWH_Transpose.cc:
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
#include <tWH_Transpose.h>

#include <Transport/TH_Mem.h>
#include <TFC_InputSection/WH_Transpose.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_SubBand.h>

namespace LOFAR
{

  AH_Transpose::AH_Transpose() :
    itsWH(0),
    itsInDH1(0),
    itsInDH2(0),
    itsOutDH1(0),
    itsOutDH2(0),
    itsInCon1(0),
    itsInCon2(0),
    itsOutCon1(0),
    itsOutCon2(0)
  {
  }

  AH_Transpose::~AH_Transpose() {
  }

  void AH_Transpose::define(const KeyValueMap& kvm) {
    KeyValueMap myKvm(kvm);
    myKvm["NoWH_RSP"] = 2;
    ACC::ParameterSet myPset;
    myPset["NoWH_RSP"] = 2;

    itsInDH1 = new DH_RSP("DH_RSP1", myPset);
    itsInDH2 = new DH_RSP("DH_RSP1", myPset);
    itsOutDH1 = new DH_SubBand("DH_SubBand", 0);
    itsOutDH2 = new DH_SubBand("DH_SubBand", 1);
    itsWH = new WH_Transpose("WH_Transpose", myKvm);
    itsTH = new TH_Mem();
    
    itsInCon1 = new Connection("in1", 
			       itsInDH1, 
			       itsWH->getDataManager().getInHolder(0), 
			       itsTH, 
			       false);
    itsWH->getDataManager().setInConnection(0, itsInCon1);
    itsInCon2 = new Connection("in2", 
			       itsInDH2, 
			       itsWH->getDataManager().getInHolder(1), 
			       itsTH, 
			       false);
    itsWH->getDataManager().setInConnection(1, itsInCon2);
    itsOutCon1 = new Connection("out1", 
				itsOutDH1, 
				itsWH->getDataManager().getOutHolder(0), 
				itsTH, 
				false);
    itsWH->getDataManager().setOutConnection(0, itsOutCon1);
    itsOutCon2 = new Connection("out2", 
				itsOutDH2, 
				itsWH->getDataManager().getOutHolder(1), 
				itsTH, 
				false);
    itsWH->getDataManager().setOutConnection(1, itsOutCon2);
  }

  void AH_Transpose::init() {
    itsWH->basePreprocess();

    // Fill inDHs here

    ((DH_RSP*)itsInDH1)->resetBuffer();
    ((DH_RSP*)itsInDH2)->resetBuffer();
  }

  void AH_Transpose::run(int nsteps) {
    for (int i = 0; i < nsteps; i++) {
      itsWH->baseProcess();
    }    
  }

  void AH_Transpose::postrun() {
    // check outresult here
    // do an assert or exit(1) if results are not correct
    ASSERTSTR(false, "no test defined yet");
  }

  void AH_Transpose::undefine() {
    delete itsWH;
    delete itsInDH1;
    delete itsInDH2;
    delete itsOutDH1;
    delete itsOutDH2;
    delete itsInCon1;
    delete itsInCon2;
    delete itsOutCon1;
    delete itsOutCon2;

    itsWH = 0;
    itsInDH1 = 0;
    itsInDH2 = 0;
    itsOutDH1 = 0;
    itsOutDH2 = 0;
    itsInCon1 = 0;
    itsInCon2 = 0;
    itsOutCon1 = 0;
    itsOutCon2 = 0;
  }
  
  void AH_Transpose::quit() {
  }

} // namespace LOFAR

using namespace LOFAR;

int main (int argc, const char** argv){

  try {
    AH_Transpose test;
    test.setarg(argc,argv);
    test.baseDefine();
    test.basePrerun();
    test.baseRun(1);
    test.basePostrun();
    test.baseQuit();

  } catch (LOFAR::Exception e) {
    cerr << "Caught exception: "<< e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception "<< endl;
    exit(1);
  }
  return 0;
}
