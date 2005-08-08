//#  tWH_SBCollect.cc:
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
#include <tWH_SBCollect.h>

#include <Transport/TH_Mem.h>
#include <TFC_InputSection/WH_SBCollect.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_FIR.h>
#include <TFC_Interface/RectMatrix.h>

namespace LOFAR
{

  AH_SBCollect::AH_SBCollect() :
    itsWH(0),
    itsInDH1(0),
    itsInDH2(0),
    itsOutDH1(0),
    itsInCon1(0),
    itsInCon2(0),
    itsOutCon1(0)
  {
  }

  AH_SBCollect::~AH_SBCollect() {
  }

  void AH_SBCollect::define(const KeyValueMap& kvm) {
    ACC::APS::ParameterSet myPset;
    myPset.add("Input.NRSP", "2");
    myPset.add("Input.NSamplesToDH", "10");
    myPset.add("Input.NPolarisations", "2");

    cout<<myPset<<endl;

    itsInDH1 = new DH_RSP("DH_RSP1", myPset);
    itsInDH2 = new DH_RSP("DH_RSP2", myPset);
    itsOutDH1 = new DH_FIR("DH_FIR", 0, myPset);
    itsWH = new WH_SBCollect("WH_SBCollect", 1, myPset);
    itsTHs.push_back(new TH_Mem());
    itsTHs.push_back(new TH_Mem());
    itsTHs.push_back(new TH_Mem());
    
    itsInCon1 = new Connection("in1", 
			       itsInDH1, 
			       itsWH->getDataManager().getInHolder(0), 
			       itsTHs[0], 
			       false);
    itsWH->getDataManager().setInConnection(0, itsInCon1);
    itsInCon2 = new Connection("in2", 
			       itsInDH2, 
			       itsWH->getDataManager().getInHolder(1), 
			       itsTHs[1], 
			       false);
    itsWH->getDataManager().setInConnection(1, itsInCon2);
    itsOutCon1 = new Connection("out1", 
				itsWH->getDataManager().getOutHolder(0), 
				itsOutDH1, 
				itsTHs[2], 
				false);
    itsWH->getDataManager().setOutConnection(0, itsOutCon1);
  }

  void AH_SBCollect::init() {
    itsWH->basePreprocess();

    // Fill inDHs here

    itsInDH1->init();
    ((DH_RSP*)itsInDH1)->resetBuffer();
    itsInDH2->init();
    ((DH_RSP*)itsInDH2)->resetBuffer();
    itsOutDH1->init();

    int value = 0;
    RectMatrix<DH_RSP::BufferType>* inMatrix = &((DH_RSP*)itsInDH1)->getDataMatrix();
    dimType timeDim = inMatrix->getDim("Times");
    RectMatrix<DH_RSP::BufferType>::cursorType tCursor = inMatrix->getCursor();
    MATRIX_FOR_LOOP(*inMatrix, timeDim, tCursor) {
      inMatrix->setValue(tCursor, makei16complex(1, value++));
    }
    inMatrix = &((DH_RSP*)itsInDH2)->getDataMatrix();
    tCursor = inMatrix->getCursor();
    MATRIX_FOR_LOOP(*inMatrix, timeDim, tCursor) {
      inMatrix->setValue(tCursor, makei16complex(2, value++));
    }
  }

  void AH_SBCollect::run(int nsteps) {
    for (int i = 0; i < nsteps; i++) {
      itsInCon1->write();
      itsInCon2->write();      
      itsWH->baseProcess();
      itsOutDH1->unpack();
      itsOutCon1->read();
    }    
  }

  void AH_SBCollect::postrun() {
    // check outresult here
    // do an assert or exit(1) if results are not correct

    // check output dhs
    int expValue = 0;
    RectMatrix<DH_FIR::BufferType>* outMatrix = &((DH_FIR*)itsOutDH1)->getDataMatrix();
    dimType timeDim = outMatrix->getDim("Time");
    RectMatrix<DH_FIR::BufferType>::cursorType tCursor = outMatrix->getCursor();
    MATRIX_FOR_LOOP(*outMatrix, timeDim, tCursor) {
      DH_FIR::BufferType value = outMatrix->getValue(tCursor);
      cout<<" e,r = " << expValue++ << "," << imag(value) << endl;
      //      ASSERTSTR(imag(value) == expValue++, "value was wrong: received "<<imag(value)<<" expected "<<expValue);
    }
  }

  void AH_SBCollect::undefine() {
    delete itsWH;
    delete itsInDH1;
    delete itsInDH2;
    delete itsOutDH1;
    delete itsInCon1;
    delete itsInCon2;
    delete itsOutCon1;

    itsWH = 0;
    itsInDH1 = 0;
    itsInDH2 = 0;
    itsOutDH1 = 0;
    itsInCon1 = 0;
    itsInCon2 = 0;
    itsOutCon1 = 0;
}  

  void AH_SBCollect::quit() {
  }

} // namespace LOFAR

using namespace LOFAR;

int main (int argc, const char** argv){

  try {
    AH_SBCollect test;
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
