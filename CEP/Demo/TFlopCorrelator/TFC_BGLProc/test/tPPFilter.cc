//#  filename.cc: one line description
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

//# Includes
#include <ACC/ParameterSet.h>
#include <tPPFilter.h>

#include <Transport/TH_Mem.h>
#include <TFC_BGLProc/WH_FIR.h>
#include <TFC_BGLProc/WH_FFT.h>
#include <TFC_Interface/DH_FIR.h>
#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>

namespace LOFAR
{

  AH_PPFilter::AH_PPFilter() :
    itsWHs(0)
  {}

  AH_PPFilter::~AH_PPFilter() {
  }

  void AH_PPFilter::define(const KeyValueMap& kvm) {
    KeyValueMap myKvm(kvm);

    itsFilterWH0 = (WorkHolder*) new WH_FIR("itsFilterWH0",0);
    itsFilterWH1 = (WorkHolder*) new WH_FIR("itsFilterWH1",0);

    itsWHs.push_back(itsFilterWH0);
    itsWHs.push_back(itsFilterWH1);
     
    itsFFTWH0 = (WorkHolder*) new WH_FFT("itsFFTWH0");
    itsFFTWH1 = (WorkHolder*) new WH_FFT("itsFFTWH1");

    itsWHs.push_back(itsFFTWH0);
    itsWHs.push_back(itsFFTWH1);

    itsInDH0 = (DataHolder*) new DH_FIR("itsInDH0", 0);
    itsInDH1 = (DataHolder*) new DH_FIR("itsInDH1", 0);

//     itsInternalDH00 = (DataHolder*) new DH_PPF("itsInternalDH00", 0);
//     itsInternalDH01 = (DataHolder*) new DH_PPF("itsInternalDH01", 0);
//     itsInternalDH10 = (DataHolder*) new DH_PPF("itsInternalDH10", 0);
//     itsInternalDH11 = (DataHolder*) new DH_PPF("itsInternalDH11", 0);
    
    itsOutDH0 = (DataHolder*) new DH_CorrCube("itsOutDH0",0);
    itsOutDH1 = (DataHolder*) new DH_CorrCube("itsOutDH1",0);

    itsTH = new TH_Mem();

    itsInCon0 = new Connection ("in0", 
				itsInDH0,
				itsFilterWH0->getDataManager().getInHolder(0),
				itsTH, 
				false);

    itsInCon1 = new Connection ("in1",
				itsInDH1,
				itsFilterWH1->getDataManager().getInHolder(0),
				itsTH, 
				false);
    
    itsInternalCon00 = new Connection("interal00",
				      itsFilterWH0->getDataManager().getOutHolder(0), 
				      itsFFTWH0->getDataManager().getInHolder(0),
				      itsTH,
				      false);
    itsInternalCon01 = new Connection("internal01",
				      itsFilterWH0->getDataManager().getOutHolder(1),
				      itsFFTWH1->getDataManager().getInHolder(0),
				      itsTH,
				      false);
    itsInternalCon10 = new Connection("internal10",
				      itsFilterWH1->getDataManager().getOutHolder(0),
				      itsFFTWH0->getDataManager().getInHolder(1),
				      itsTH,
				      false);
    itsInternalCon11 = new Connection("internal11",
				      itsFilterWH1->getDataManager().getOutHolder(1),
				      itsFFTWH1->getDataManager().getInHolder(1),
				      itsTH,
				      false);

    
    itsOutCon0 = new Connection("out0",
				itsFFTWH0->getDataManager().getOutHolder(0),
				itsOutDH0, 
				itsTH, 
				false);
    itsOutCon1 = new Connection("out1",
				itsFFTWH1->getDataManager().getOutHolder(1),
				itsOutDH1,
				itsTH,
				false);
  }


  void AH_PPFilter::undefine() {
  }

  void AH_PPFilter::init() {
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for(; it != itsWHs.end(); it++) {
      (*it)->basePreprocess();
    }
  }

  void AH_PPFilter::run(int nsteps) {
    vector<WorkHolder*>::iterator it = itsWHs.begin();
    for (int i = 0; i < nsteps; i++) {
      for(; it != itsWHs.end(); it++) {
	(*it)->baseProcess();
      }
    }
  }
  
  void AH_PPFilter::postrun() {
  }

  void AH_PPFilter::quit() {
    delete itsFilterWH0;
    delete itsFilterWH1;
    delete itsFFTWH0;
    delete itsFFTWH1;
    
    delete itsInDH0;
    delete itsInDH1;
    delete itsOutDH0;
    delete itsOutDH1;
    
    delete itsInCon0;
    delete itsInCon1;
    delete itsInternalCon00;
    delete itsInternalCon01;
    delete itsInternalCon10;
    delete itsInternalCon11;
    delete itsOutCon0;
    delete itsOutCon1;

    delete itsTH;
  }

} // namespace LOFAR

using namespace LOFAR;

int main (int argc, const char** argv) {
  
  try {

    AH_PPFilter test;
    test.setarg(argc, argv);
    test.baseDefine();
    test.basePrerun();
    test.baseRun(1);
    test.basePostrun();
    test.baseQuit();

  } catch (LOFAR::Exception e) {
    cerr <<  "Caught exception: "<< e.what()<< endl;
    exit(1);
  } catch (...){ 
    cerr << "Caught non-LOFAR exception." << endl;
    exit(1);
  }
  return 0;

}
