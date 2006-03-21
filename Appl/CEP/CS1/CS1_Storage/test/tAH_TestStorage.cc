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
#include <CEPFrame/Step.h>
#include <Transport/TH_Mem.h>
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CS1_Interface/DH_Visibilities.h>

namespace LOFAR
{

  AH_TestStorage::AH_TestStorage()
  {
  }

  AH_TestStorage::~AH_TestStorage() {
    undefine();
  }

  void AH_TestStorage::define(const KeyValueMap&) {

    LOG_TRACE_FLOW_STR("Start of tAH_Storage::define()");

    LOG_TRACE_FLOW_STR("Create the top-level composite");
    Composite comp(0, 0, "topComposite");
    setComposite(comp); // tell the ApplicationHolder this is the top-level composite

    int nrSubbands = itsParamSet.getInt32("Observation.NSubbands");

    for (int subb=0; subb< nrSubbands; subb++)
    {
      WH_SubbandWriter wh("storage1", subb, itsParamSet);
      Step step(wh);
      comp.addBlock(step);
      
      for (int nr=0; nr < step.getNrInputs(); nr++)
      {
	DH_Visibilities* inDH = new DH_Visibilities("in_"+nr, itsParamSet);
	itsInDHs.push_back(inDH);

	Connection* inConn = new Connection("in_"+nr, 
					    inDH,
					    step.getInDataManager(nr).getGeneralInHolder(nr),
					    new TH_Mem(),
					    false);
	itsInConns.push_back(inConn);
	step.getInDataManager(nr).setInConnection(nr, inConn);
      }
	 
    }
    
    LOG_TRACE_FLOW_STR("Finished define()");

  }
  
  void AH_TestStorage::prerun() {
    getComposite().preprocess();

    // Fill inDHs here
    for (uint i = 0; i < itsInDHs.size(); i++)
    {
      itsInDHs[i]->init();
      itsInDHs[i]->setStorageTestPattern(i);
    }
  }

  void AH_TestStorage::run(int nsteps) {

    for (int i = 0; i < nsteps; i++) {
      for (uint nrInp = 0; nrInp < itsInConns.size(); nrInp++)
      {
	itsInConns[nrInp]->write();
      }

      LOG_TRACE_LOOP_STR("processing run " << i );
      cout<<"run "<<i+1<<" of "<<nsteps<<endl;
      getComposite().process();
      
    }    
  }

  void AH_TestStorage::postrun() {
    // check outresult here
    // do an assert or exit(1) if results are not correct

   }

  void AH_TestStorage::undefine() {
    for (uint i = 0; i < itsInDHs.size(); i++)
    {
      delete itsInDHs[i];
      delete itsInConns[i];
    }
    itsInDHs.clear();
    itsInConns.clear();
  }  

  void AH_TestStorage::quit() {
  }

} // namespace LOFAR

using namespace LOFAR;

int main (int argc, const char** argv){
  INIT_LOGGER("TestStorage");
  try {
    AH_TestStorage test;
    ACC::APS::ParameterSet ps("CS1.cfg");
    test.setParameters(ps);
    test.setarg(argc,argv);
    test.baseDefine();
    test.basePrerun();
    int nrRuns = ps.getInt32("General.NRuns");
    cout << "run " << nrRuns << " times" << endl;
    test.baseRun(nrRuns);
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
