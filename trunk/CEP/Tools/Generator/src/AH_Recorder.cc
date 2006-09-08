//#  AH_Recorder.cc: one line description
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
#include <Generator/AH_Recorder.h>
#include <Generator/RSPTimeStamp.h>
#include <Common/lofar_iostream.h>
#include <APS/ParameterSet.h>
#include <CEPFrame/Step.h>

// Transporters
#include <Transport/TH_Mem.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Null.h>
#include <Transport/TH_Ethernet.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <Generator/WH_Signal.h>
#include <Generator/WH_FakeStation.h>

namespace LOFAR {
  namespace Generator {

    AH_Recorder::AH_Recorder() 
    {
    }

    AH_Recorder::~AH_Recorder() 
    {
      this->undefine();
    }

    void AH_Recorder::undefine() {
      vector<TransportHolder*>::iterator tit = itsTHs.begin();
      for (; tit!=itsTHs.end(); tit++) {
	delete *tit;
      }
      itsTHs.clear();
    }  

    void AH_Recorder::define(const LOFAR::KeyValueMap&) {
      LOG_TRACE_FLOW_STR("Start of AH_Recorder::define()");
      undefine();
    
      LOG_TRACE_FLOW_STR("Create the top-level composite");
      Composite comp(0, 0, "topComposite");
      setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

      int lastFreeNode = 0;
#ifdef HAVE_MPICH
      // mpich tries to run the first process on the local node
      // this is necessary if you want to use totalview
      // scampi run the first process on the first node in the machinefile
      lastFreeNode = 1;
#endif

      TimeStamp::setMaxBlockId(itsParamSet.getDouble("Generator.SampleFreq"));


      int NRSP = itsParamSet.getInt32("Data.NStations");
      int WH_DH_NameSize = 40;
      char WH_DH_Name[WH_DH_NameSize];
      vector<string> outFileNames = itsParamSet.getStringVector("Generator.OutputFiles");
      vector<string> interfaces = itsParamSet.getStringVector("Input.Interfaces");
      vector<string> dstMacs = itsParamSet.getStringVector("Input.DestinationMacs");
      vector<string> srcMacs = itsParamSet.getStringVector("Input.SourceMacs");
      int bufferSize = itsParamSet.getInt32("Generator.RecordBufferSize");
  
      snprintf(WH_DH_Name, WH_DH_NameSize, "Signal");

#if 0
      for (int s=0; s<NRSP; s++) {
	//itsTHs.push_back(new TH_File("Generator1.in", TH_File::Read));
	cout<<"Creating TH_Ethernet: "<<srcMacs[s]<<" -> "<<dstMacs[s]<<endl;

#if 0
	itsTHs.push_back(new TH_Null());
#else
	itsTHs.push_back(new TH_Ethernet(interfaces[s],
					 srcMacs[s],
					 dstMacs[s],
					 1048576));
#endif
	Step inStep(&WH_Wrap(WH_DH_Name,
			     *itsTHs.back(),
			     itsParamSet), 
		    WH_DH_Name);
	inStep.setOutBuffer(0, false, bufferSize);
	comp.addBlock(inStep);
	inStep.runOnNode(lastFreeNode++);

	itsTHs.push_back(new TH_File(outFileNames[s], TH_File::Write));

	Step outStep(&WH_Strip(WH_DH_Name,
			       *itsTHs.back(),
			       itsParamSet), 
		     WH_DH_Name);
	comp.addBlock(outStep);
	outStep.runOnNode(lastFreeNode++);
    
#ifdef HAVE_MPI
	// this needs to be done with MPI, so if we don't have MPI do nothing
	outStep.connect(0, inStep, 0, 1,
			TH_MPI(inStep->getNode(),
				   outStep->getNode()),
			true);
#else
	ASSERTSTR(false, "This application is supposed to be run with MPI");
#endif
      }
#endif
      // This program was written to run with MPI. All workholders run in their own process.
      // The machinefile should contain every node name twice, so WH_Wrap and WH_Strip run on 
      // the same physical host.
#ifdef HAVE_MPI
      ASSERTSTR (lastFreeNode == TH_MPI::getNumberOfNodes(), lastFreeNode << " nodes needed, "<<TH_MPI::getNumberOfNodes()<<" available");
#else
      ASSERTSTR(false, "This application is supposed to be run with MPI");
#endif

      LOG_TRACE_FLOW_STR("Finished defineRecorder()");
    }

    void AH_Recorder::run(int steps) {
      LOG_TRACE_FLOW_STR("Start AH_Recorder::run() "  );
      for (int i = 0; i < steps; i++) {
	LOG_TRACE_LOOP_STR("processing run " << i );
	getComposite().process();
      }
      LOG_TRACE_FLOW_STR("Finished AH_Recorder::run() "  );
    }

  } // namespace Generator
} // namespace LOFAR
