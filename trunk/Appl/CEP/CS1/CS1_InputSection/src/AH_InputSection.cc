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
#include <CS1_Interface/RSPTimeStamp.h>

//# Workholders
#include <CS1_InputSection/WH_RSPInput.h>
#include <CS1_InputSection/WH_SBCollect.h>

#include <Transport/TransportHolder.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_Socket.h>
#include <Transport/TH_File.h>

#define IS_MULTIPLE(number, bignumber) (floor(bignumber / number) == (1.0 * bignumber / number))

namespace LOFAR {
  namespace CS1 {

    AH_InputSection::AH_InputSection() :
      itsOutputStub(0),
      itsInputStub(0)
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

      int lowestFreeNode = 0;
      
      TimeStamp::setMaxBlockId(itsParamSet.getDouble("Observation.SampleRate"));

      int psetsPerCell = itsParamSet.getInt32("BGLProc.PsetsPerCell");
      int nCells  = itsParamSet.getInt32("Observation.NSubbands") / (itsParamSet.getInt32("General.SubbandsPerPset") * psetsPerCell);  // number of SubBand filters in the application
      int nNodesPerCell = itsParamSet.getInt32("BGLProc.NodesPerPset") * psetsPerCell;
    
      LOG_TRACE_FLOW_STR("Create the top-level composite");
      Composite comp(0, 0, "topComposite");
      setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

      LOG_TRACE_FLOW_STR("Create the input side delay stub");
      // TODO create connector class

      LOG_TRACE_FLOW_STR("Create the RSP reception Steps");
  

      int nRSP = itsParamSet.getInt32("Input.NRSPBoards");
      int nStations = itsParamSet.getInt32("Observation.NStations");
      int inputCells = nRSP/nStations;
      int nameBufferSize = 40;
      char nameBuffer[nameBufferSize];

      vector<string> outputFileNames;
      bool writeRawDataToFile = itsParamSet.getBool("Input.WriteRawDataToFile");
      if (!writeRawDataToFile)
      {
        itsOutputStub = new Stub_BGL_Subband(false, itsParamSet);
      }
      else
      {
        outputFileNames = itsParamSet.getStringVector("Input.OutputRawDataFiles");
        nNodesPerCell = (itsParamSet.getInt32("General.SubbandsPerPset") * psetsPerCell);
      }
       
      itsInputStub = new Stub_Delay(true, itsParamSet);

      for (int ic = 0; ic < inputCells; ic ++) {
	WorkHolder* lastWH;
	vector<Step*>        RSPSteps;
	for (int station = 0; station < nStations; station ++) {
	  snprintf(nameBuffer, nameBufferSize, "Input.Transport.Station%d.Rsp%d", station, ic);
	  TransportHolder* lastTH = Connector::readTH(itsParamSet, nameBuffer, true); 
    
	  snprintf(nameBuffer, nameBufferSize, "RSP_Input_node_station%d_cell%d", station, ic);
	  lastWH = new WH_RSPInput(nameBuffer,
				   itsParamSet,
				   *lastTH,
				   station);
	  RSPSteps.push_back(new Step(lastWH, nameBuffer, false));
#ifdef HAVE_MPI
	  RSPSteps.back()->runOnNode(lowestFreeNode++);   
#endif
	  comp.addBlock(RSPSteps.back());
    
	  // Connect the Delay Controller
	  //itsInputStub->connect(ic * nStations + station, (RSPSteps.back())->getInDataManager(0), 0);
	}

	LOG_TRACE_FLOW_STR("Create the Subband merger workholders");
	vector<Step*> collectSteps;
	for (int cell = 0; cell < nCells / inputCells; cell++) {
	  sprintf(nameBuffer, "Collect_node_%d_%d", cell, ic);
	  lastWH = new WH_SBCollect(nameBuffer,      // name
				    itsParamSet,
				    nNodesPerCell);
	  collectSteps.push_back(new Step(lastWH, nameBuffer, false));
#ifdef HAVE_MPI
	  collectSteps.back()->runOnNode(lowestFreeNode++); 
#endif
	  comp.addBlock(collectSteps.back());

	  // Connect splitters to mergers (transpose)
#ifndef HAVE_MPI
	  for (int station = 0; station < nStations; station++) {
	    itsConnector.connectSteps(RSPSteps[station], cell, collectSteps.back(), station);
	  }
#else
#if MPICH_WORKING_ON_INFINI_BAND
	  for (int station = 0; station < nStations; station++) {
	    itsConnector.connectSteps(RSPSteps[station], cell, collectSteps.back(), station);
	  }
#else
	  vector<string> transposeHosts = itsParamSet.getStringVector("TransposeHosts");
	  vector<string> transposePorts = itsParamSet.getStringVector("TransposePorts");

	  for (int station = 0; station < nStations; station++) {
	    // We need to find out if we are on the client or the server
	    // because TH_Socket doesn't find it out itself.
	    if (collectSteps.back()->getNode() == TH_MPI::getCurrentRank()) {
	      // create server socket
	      collectSteps.back()->connect(station,
					   RSPSteps[station],
					   cell,
					   1,
					   new TH_Socket(transposePorts[station],
							 true,
							 Socket::TCP,
							 5,
							 false),
					   false);
	    } else {
	      // create client socket
	      collectSteps.back()->connect(station,
					   RSPSteps[station],
					   cell,
					   1,
					   new TH_Socket(transposeHosts[cell],
							 transposePorts[station],
							 true,
							 Socket::TCP,
							 false),
					   false);	      
	    }
	  }
#endif
#endif
	  // connect outputs to Subband stub
	  vector<int> channels;
	  bool makeConnections = itsParamSet.getBool("Connect");
	  if (makeConnections) {
	    for (int core = 0; core < nNodesPerCell; core++) {
	      collectSteps.back()->getOutDataManager(0).setOutBuffer(core, false, 3);
	      if (writeRawDataToFile)
              {
	        TransportHolder *theTH = 0;
		
	        theTH = new TH_File(outputFileNames[core], TH_File::Write);
	        collectSteps.back()->connectOut("output", theTH, core, true);
	      }
	      else
	      { 
	        itsOutputStub->connect(cell + ic * nCells / inputCells,
				       core,
				       (collectSteps.back())->getOutDataManager(0), 
				       core);
		
		channels.push_back(core);
              } 
	    }
	    if (!writeRawDataToFile)
	    {
	      collectSteps.back()->getOutDataManager(0).setOutRoundRobinPolicy(channels, itsParamSet.getInt32("BGLProc.MaxConcurrentCommunications"));
	    }  
	  }
	}
      }
      
      LOG_TRACE_FLOW_STR("Finished define()");

#ifdef HAVE_MPI
      ASSERTSTR (lowestFreeNode == TH_MPI::getNumberOfNodes(), "CS1_InputSection needs "<< lowestFreeNode << " nodes, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif
    }

    void AH_InputSection::prerun() {
      getComposite().preprocess();
    }
    
    void AH_InputSection::run(int steps) {
      LOG_TRACE_FLOW_STR("Start AH_InputSection::run() "  );
      for (int i = 0; i < steps; i++) {
	LOG_TRACE_LOOP_STR("processing run " << i );
	getComposite().process();
      }
      LOG_TRACE_FLOW_STR("Finished AH_InputSection::run() "  );
    }

    void AH_InputSection::dump() const {
      LOG_TRACE_FLOW_STR("AH_InputSection::dump() not implemented"  );
    }

    void AH_InputSection::quit() {
    }

  } // namespace CS1
} // namespace LOFAR
