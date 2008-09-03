//#  AH_Generator.cc: one line description
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
#include <Generator/AH_Generator.h>
#include <Generator/RSPTimeStamp.h>
#include <Common/lofar_iostream.h>
#include <APS/ParameterSet.h>
#include <CEPFrame/Step.h>
#include <Generator/Connector.h>

// Transporters
#include <Transport/TH_Mem.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Ethernet.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <Generator/WH_Signal.h>
#include <Generator/WH_FakeStation.h>


namespace LOFAR {
  namespace Generator {

    AH_Generator::AH_Generator() 
    {
    }

    AH_Generator::~AH_Generator() 
    {
      this->undefine();
    }

    void AH_Generator::undefine() {
      vector<TransportHolder*>::iterator tit = itsTHs.begin();
      for (; tit!=itsTHs.end(); tit++) {
	delete *tit;
      }
      itsTHs.clear();
    }  

    void AH_Generator::define(const LOFAR::KeyValueMap&) {
      LOG_TRACE_FLOW_STR("Start of AH_Generator::define()");
      undefine();
    
      LOG_TRACE_FLOW_STR("Create the top-level composite");
      Composite comp(0, 0, "topComposite");
      setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

      LOG_TRACE_FLOW_STR("Create output side interface stubs");
      // todo: define this input interface; although there are no
      //       connection involved, we do have to define the port/IP numbering schemes

      TimeStamp::setMaxBlockId(itsParamSet.getDouble("Generator.SampleFreq"));

      int NRSP = itsParamSet.getInt32("Generator.NStations");
      int WH_DH_NameSize = 40;
      char WH_DH_Name[WH_DH_NameSize];
  
      snprintf(WH_DH_Name, WH_DH_NameSize, "Signal");

      Step signalStep(new WH_Signal(WH_DH_Name, 
				    NRSP,
				    itsParamSet), 
		      WH_DH_Name);
      comp.addBlock(signalStep);

      for (int s=0; s<NRSP; s++) {
	snprintf(WH_DH_Name, WH_DH_NameSize, "Generator.Station%d", s);
	itsTHs.push_back(Connector::readTH(itsParamSet, WH_DH_Name, false));

	snprintf(WH_DH_Name, WH_DH_NameSize, "Generator.Station%d.ID", s);
	int stationId = itsParamSet.getInt32(WH_DH_Name);

	snprintf(WH_DH_Name, WH_DH_NameSize, "Generator.Station%d.Delay", s);
	int delay = itsParamSet.getInt32(WH_DH_Name);

	ASSERTSTR(itsTHs.back()->init(), "Could not init TransportHolder");
	snprintf(WH_DH_Name, WH_DH_NameSize, "FakeStation_%d_of_%d", s, NRSP);
	Step stationStep(new WH_FakeStation(WH_DH_Name,
					    itsParamSet,
					    stationId,
					    delay,
					    itsTHs.back()), 
			 WH_DH_Name);
	// share input and output DH, no cyclic buffer
	stationStep.setInBufferingProperties(0, true, true);
	
	comp.addBlock(stationStep);
	stationStep.connect(0, &signalStep, s, 1,
			    new TH_Mem(),
			    false);
	
	snprintf(WH_DH_Name, WH_DH_NameSize, "Strip_%d_of_%d", s, NRSP);
      };

#ifdef HAVE_MPI
      // TODO How do we want to distribute the steps across the nodes?
      //  ASSERTSTR (lastFreeNode == TH_MPI::getNumberOfNodes(), lastFreeNode << " nodes needed, "<<TH_MPI::getNumberOfNodes()<<" available");
#endif
      
      LOG_TRACE_FLOW_STR("Finished defineGenerator()");
    }
    
    void AH_Generator::run(int steps) {
      LOG_TRACE_FLOW_STR("Start AH_Generator::run() "  );
      for (int i = 0; i < steps; i++) {
	LOG_TRACE_LOOP_STR("processing run " << i );
	getComposite().process();
      }
      LOG_TRACE_FLOW_STR("Finished AH_Generator::run() "  );
    }

  } // namespace Generator
} // namespace LOFAR
