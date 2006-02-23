//#  AH_InputSection.cc: one line description
//#
//#  Copyright (C) 2006
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
#include <Common/LofarLogger.h>
#include <CS1_InputSection/AH_InputSection.h>

//# Workholders
#include <CS1_InputSection/WH_RSPInput.h>
#include <CS1_InputSection/WH_SBCollect.h>

#include <Transport/TransportHolder.h>
#include <Transport/TH_MPI.h>

namespace LOFAR {
  namespace CS1_InputSection {

    AH_InputSection::AH_InputSection()
    {}

    AH_InputSection::~AH_InputSection()
    {
      undefine();
    }

    void AH_InputSection::undefine()
    {
      delete itsOutputStub;
      delete itsInputStub;
      itsInputStub = 0;
      itsOutputStub = 0;
    }

    void AH_InputSection::define(const LOFAR::KeyValueMap&) 
    {
      LOG_TRACE_FLOW_STR("Start of AH_InputSection::define()");
      undefine();

#ifdef HAVE_MPICH
      // mpich needs to run the first process on the master node to enable debugging
      int lowestFreeNode = 1;
#else
      // scampi doesn't need an extra node
      int lowestFreeNode = 0;
#endif
      
      int nSubbands  = itsParamSet.getInt32("Data.NSubbands");  // number of SubBand filters in the application
      int nCoresPerSubband = itsParamSet.getInt32("BGLProc.SlavesPerSubband");
    
      LOG_TRACE_FLOW_STR("Create the top-level composite");
      Composite comp(0, 0, "topComposite");
      setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

      LOG_TRACE_FLOW_STR("Create the input side delay stub");
      // TODO create connector class

      LOG_TRACE_FLOW_STR("Create the RSP reception Steps");
  
      WorkHolder* lastWH;
      vector<Step*>        RSPSteps;

      int nStations = itsParamSet.getInt32("Data.NStations");
      int nameBufferSize = 40;
      char nameBuffer[nameBufferSize];
      int rspStartNode;
  
      int nOutputsPerSubband = itsParamSet.getInt32("FakeData.NSubbands") / itsParamSet.getInt32("Data.NSubbands"); 
    
      itsOutputStub = new Stub_BGL_Subband(false, itsParamSet);
      itsInputStub = new Stub_CoarseDelay(true, itsParamSet);


      for (int r=0; r<nStations; r++) {
   
	// TODO: we could use a connector here too
	snprintf(nameBuffer, nameBufferSize, "Input.Transport.%d", r);
	TransportHolder* lastTH = Connector::readTH(itsParamSet, nameBuffer, true); 
    
	snprintf(nameBuffer, nameBufferSize, "RSP_Input_node_%d_of_%d", r, nStations);
	if (r==0) {
	  lastWH = new WH_RSPInput(nameBuffer,  // create sync master
				   itsParamSet,
				   *lastTH,
				   true);
	  rspStartNode = lowestFreeNode;
	} else {
	  lastWH = new WH_RSPInput(nameBuffer,  // create slave
				   itsParamSet,
				   *lastTH,
				   false);
	}
	RSPSteps.push_back(new Step(lastWH, nameBuffer, false));
	RSPSteps[r]->runOnNode(lowestFreeNode++);   
	comp.addBlock(RSPSteps[r]);
    
	// Connect the Delay Controller
	itsInputStub->connect(r, (RSPSteps.back())->getInDataManager(0), 0);
	if (r!=0) {
	  itsConnector.connectSteps(RSPSteps[0], nSubbands + r - 1, RSPSteps.back(), 1);
	}
      }
  
      LOG_TRACE_FLOW_STR("Create output side interface stubs");

      LOG_TRACE_FLOW_STR("Create the Subband merger workholders");
      vector<Step*> collectSteps;
      for (int nf=0; nf < nSubbands; nf++) {
	sprintf(nameBuffer, "Collect_node_%d_of_%d", nf, nStations);
	lastWH = new WH_SBCollect(nameBuffer,      // name
				  nf,              // Subband ID
				  itsParamSet,
				  nCoresPerSubband);
	collectSteps.push_back(new Step(lastWH, nameBuffer, false));
	collectSteps.back()->runOnNode(lowestFreeNode++); 
	comp.addBlock(collectSteps.back());

	// Connect splitters to mergers (transpose)
	for (int st=0; st<nStations; st++) {
	  itsConnector.connectSteps(RSPSteps[st], nf, collectSteps.back(), st);
	}
	// connect outputs to FIR stub
	for (int core = 0; core < nCoresPerSubband; core++) {
	  itsOutputStub->connect(nf,
			     core,
			     (collectSteps.back())->getOutDataManager(core), 
			     core);
	}
      }
      LOG_TRACE_FLOW_STR("Finished define()");

#ifdef HAVE_MPI
      ASSERTSTR (lowestFreeNode == TH_MPI::getNumberOfNodes(), "TFC_InputSection needs "<< lowestFreeNode << " nodes, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif
    }

    void AH_InputSection::prerun() {
      getComposite().preprocess();
    }
    
    void AH_InputSection::run(int steps) {
      LOG_TRACE_FLOW_STR("Start AH_InputSection::run() "  );
      for (int i = 0; i < steps; i++) {
	LOG_TRACE_LOOP_STR("processing run " << i );
	cout<<"run "<<i+1<<" of "<<steps<<endl;
	getComposite().process();
      }
      LOG_TRACE_FLOW_STR("Finished AH_InputSection::run() "  );
    }

    void AH_InputSection::dump() const {
      LOG_TRACE_FLOW_STR("AH_InputSection::dump() not implemented"  );
    }

    void AH_InputSection::quit() {
    }

  } // namespace CS1_InputSection
} // namespace LOFAR
