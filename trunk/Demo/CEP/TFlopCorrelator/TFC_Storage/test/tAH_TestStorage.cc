//#  tAH_TestStorage.cc:
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
#include <tAH_TestStorage.h>

#include <Transport/TH_Mem.h>
#include <TFC_Storage/WH_Storage.h>
#include <TFC_Interface/DH_VisArray.h>

namespace LOFAR
{

  AH_TestStorage::AH_TestStorage() :
    itsWH(0),
    itsInDH1(0),
    itsInDH2(0),
    itsInCon1(0),
    itsInCon2(0),
    itsNStations(0),
    itsNChannels(0)
  {
  }

  AH_TestStorage::~AH_TestStorage() {
    undefine();
  }

  void AH_TestStorage::define(const KeyValueMap& kvm) {
    ACC::APS::ParameterSet myPset;
    myPset.add("Input.NPolarisations", "2");
    myPset.add("PPF.NrPolarizations", "2");
    myPset.add("BGLProc.NrStoredSubbands", "2");
    myPset.add("PPF.NrStations", "2");
    myPset.add("Storage.WriteToMAC", "F");
    myPset.add("Storage.MSName", "TestPattern.MS");
    myPset.add("Storage.refFreqs", "[1.0e8, 2.0e8]");
    myPset.add("PPF.NrSubChannels", "6");
    itsNChannels = 6;
    myPset.add("Storage.NVisPerInput", "6");
    myPset.add("Storage.chanWidth", "1.0e8");
    myPset.add("Storage.startTime", "123");
    myPset.add("Storage.timeStep", "1");
    itsNStations = 2;
    myPset.add("Storage.stationPositions", "[0,0,0, 1,1,1]");    
    myPset.add("Storage.beamAzimuth", "0.0");
    myPset.add("Storage.beamElevation", "90.0");
    myPset.add("Storage.pi", "3.14159265");

    cout << myPset << endl;

    itsInDH1 = new DH_VisArray("DH_VisArray1", myPset);
    itsInDH2 = new DH_VisArray("DH_VisArray2", myPset);
    itsWH = new WH_Storage("WH_Storage",myPset);
    
    itsInCon1 = new Connection("in1", 
			       itsInDH1, 
			       itsWH->getDataManager().getInHolder(0), 
			       new TH_Mem(), 
			       false);
    itsWH->getDataManager().setInConnection(0, itsInCon1);
    itsInCon2 = new Connection("in2", 
			       itsInDH2, 
			       itsWH->getDataManager().getInHolder(1), 
			       new TH_Mem(), 
			       false);
    itsWH->getDataManager().setInConnection(1, itsInCon2);

  }

  void AH_TestStorage::init() {
    itsWH->basePreprocess();
    // Fill inDHs here
    itsInDH1->init();
    itsInDH2->init();

    // Fill DH1
    DH_VisArray* dh1Ptr = (DH_VisArray*)itsInDH1;
    for (int ch=0; ch<itsNChannels; ch++)
    {
      fcomplex* dataPtr = dh1Ptr->getBufferElement(ch, 0,0,0);
      for (int st1=0; st1<itsNStations; st1++)
      {
	for (int st2=0; st2<=st1; st2++)
	{
	  dataPtr[0] = makefcomplex(st1, ch+100);
	  dataPtr[1] = makefcomplex(st1+10, ch+100);
	  dataPtr[2] = makefcomplex(st2, ch+100);
	  dataPtr[3] = makefcomplex(st2+10, ch+100);
	  dataPtr += 4;
	}
      }
      dh1Ptr->setCenterFreq(ch*1.0e07+1.0e08, ch);
    }
    
    // Fill DH2
    DH_VisArray* dh2Ptr = (DH_VisArray*)itsInDH2;
    for (int ch=0; ch<itsNChannels; ch++)
    {
      fcomplex* dataPtr = dh2Ptr->getBufferElement(ch, 0,0,0);
      for (int st1=0; st1<itsNStations; st1++)
      {
	for (int st2=0; st2<=st1; st2++)
	{
	  dataPtr[0] = makefcomplex(st1, ch+200);
	  dataPtr[1] = makefcomplex(st1+10, ch+200);
	  dataPtr[2] = makefcomplex(st2, ch+200);
	  dataPtr[3] = makefcomplex(st2+10, ch+200);
	  dataPtr += 4;
	}
      }
      dh2Ptr->setCenterFreq(ch*1.0e07+2.0e08, ch);
    }
  }

  void AH_TestStorage::run(int nsteps) {

    for (int i = 0; i < nsteps; i++) {
      itsInCon1->write();
      itsInCon2->write();      
      itsWH->baseProcess();
    }    
  }

  void AH_TestStorage::postrun() {
    // check outresult here
    // do an assert or exit(1) if results are not correct

   }

  void AH_TestStorage::undefine() {
    delete itsWH;
    delete itsInDH1;
    delete itsInDH2;
    delete itsInCon1;
    delete itsInCon2;
 
    itsWH = 0;
    itsInDH1 = 0;
    itsInDH2 = 0;
    itsInCon1 = 0;
    itsInCon2 = 0;
}  

  void AH_TestStorage::quit() {
  }

} // namespace LOFAR

using namespace LOFAR;

int main (int argc, const char** argv){
  INIT_LOGGER("TestStorage");
  try {
    AH_TestStorage test;
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
