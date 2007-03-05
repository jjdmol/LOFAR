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
#include <CS1_InputSection/WH_InputSection.h>
#include <CS1_Interface/RSPTimeStamp.h>

//# Workholders

#include <Transport/TransportHolder.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_Socket.h>
#include <Transport/TH_File.h>

#include <algorithm>


#define MPICH_WORKING_ON_INFINI_BAND 1

#define IS_MULTIPLE(number, bignumber) (floor(bignumber / number) == (1.0 * bignumber / number))

namespace LOFAR {
namespace CS1 {

AH_InputSection::AH_InputSection() :
  itsDelayStub(0),
  itsOutputStub(0)
{}

AH_InputSection::~AH_InputSection()
{
  undefine();
}

void AH_InputSection::undefine()
{
  delete itsOutputStub;
  delete itsDelayStub;
  itsDelayStub = 0;
  itsOutputStub = 0;
}

void AH_InputSection::define(const LOFAR::KeyValueMap&) 
{
  LOG_TRACE_FLOW_STR("Start of AH_InputSection::define()");

  itsParamSet.getDouble("Observation.SampleRate");
  TimeStamp::setMaxBlockId(itsParamSet.getDouble("Observation.SampleRate"));

  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

  LOG_TRACE_FLOW_STR("Create the input side delay stub");
  LOG_TRACE_FLOW_STR("Create the RSP reception Steps");

  itsDelayStub  = new Stub_Delay(true, itsParamSet);
  itsOutputStub = new Stub_BGL(false, false, "Input_BGLProc", itsParamSet);

  // TODO: support multiple RSPs per station
  itsInputNodes  = itsParamSet.getUint32Vector("Input.InputNodes");
  itsOutputNodes = itsParamSet.getUint32Vector("Input.OutputNodes");
  unsigned nrBGLnodesPerCell = itsParamSet.getUint32("BGLProc.NodesPerPset") * itsParamSet.getInt32("BGLProc.PsetsPerCell");

#if defined HAVE_MPI
  unsigned nrNodes = TH_MPI::getNumberOfNodes();
#else
  unsigned nrNodes = 1;
#endif

  itsWHs.resize(nrNodes);

  for (unsigned node = 0, cell = 0, station = 0; node < nrNodes; node ++) {
    bool isInput  = std::find(itsInputNodes.begin(), itsInputNodes.end(), node) != itsInputNodes.end();
    bool isOutput = std::find(itsOutputNodes.begin(), itsOutputNodes.end(), node) != itsOutputNodes.end();
    TransportHolder *th = 0;
    char nameBuffer[40];

    if (isInput) {
      snprintf(nameBuffer, sizeof nameBuffer, "Input.Transport.Station%d.Rsp%d", station, 0); // FIXME last arg is RSP number
      th = Connector::readTH(itsParamSet, nameBuffer, true); 
    }

    itsWHs[node] = new WH_InputSection("InputSection", itsParamSet, th, isInput ? station : 0, isInput ? 1 : 0, isOutput ? nrBGLnodesPerCell : 0, itsInputNodes, itsOutputNodes);
    Step *step = new Step(itsWHs[node], "Step", false);
    step->runOnNode(node); 
    comp.addBlock(step);

    if (isInput) {
      itsDelayStub->connect(station, step->getInDataManager(0), 0);
      station ++;
    }

    if (isOutput) {
      DataManager      &dm = step->getOutDataManager(0);
      std::vector<int> channels(nrBGLnodesPerCell);

      for (unsigned core = 0; core < nrBGLnodesPerCell; core ++) {
	dm.setOutBuffer(core, false, 3);
	itsOutputStub->connect(cell, core, dm, core);
	channels[core] = core;
      }
	
      dm.setOutRoundRobinPolicy(channels, itsParamSet.getInt32("BGLProc.MaxConcurrentCommunications"));
      cell ++;
    }
  }
  
  LOG_TRACE_FLOW_STR("Finished define()");
}

void AH_InputSection::run(int steps)
{
  LOG_TRACE_FLOW_STR("Start AH_InputSection::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_InputSection::run() "  );
}

} // namespace CS1
} // namespace LOFAR
