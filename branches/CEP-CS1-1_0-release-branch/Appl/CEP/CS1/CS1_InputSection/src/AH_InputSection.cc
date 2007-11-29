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
#include <CS1_InputSection/Connector.h>
#include <CS1_Interface/RSPTimeStamp.h>

//# Workholders

#include <Transport/TransportHolder.h>
#include <Transport/TH_MPI.h>

#include <algorithm>

namespace LOFAR {
namespace CS1 {

AH_InputSection::AH_InputSection() :
  itsCS1PS(0),
  itsDelayStub(0),
  itsOutputStub(0)
{
}

AH_InputSection::~AH_InputSection()
{
  undefine();
}

void AH_InputSection::undefine()
{
  for (unsigned i = 0; i < itsWHs.size(); i ++)
    delete itsWHs[i];

  itsWHs.resize(0);

  delete itsOutputStub;
  delete itsDelayStub;
  delete itsCS1PS;
  itsDelayStub = 0;
  itsOutputStub = 0;
  itsCS1PS = 0;
}

void AH_InputSection::define(const LOFAR::KeyValueMap&) 
{
  LOG_TRACE_FLOW_STR("Start of AH_InputSection::define()");
  itsCS1PS = new CS1_Parset(&itsParamSet);
  itsCS1PS->adoptFile("OLAP.parset");

   TimeStamp::setMaxBlockId(itsCS1PS->sampleRate());

  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

  LOG_TRACE_FLOW_STR("Create the input side delay stub");
  LOG_TRACE_FLOW_STR("Create the RSP reception Steps");

  itsDelayStub  = new Stub_Delay(true, itsCS1PS);
  itsOutputStub = new Stub_BGL(false, false, "input_BGLProc", itsCS1PS);

  // TODO: support multiple RSPs per station
  itsInputNodes  = itsCS1PS->getUint32Vector("Input.InputNodes");
  itsOutputNodes = itsCS1PS->getUint32Vector("Input.OutputNodes");
  unsigned nrOutputChannels = itsCS1PS->nrOutputsPerInputNode();

#if defined HAVE_MPI
  unsigned nrNodes = TH_MPI::getNumberOfNodes();
#else
  unsigned nrNodes = 1;
#endif

  itsWHs.resize(nrNodes);

  bool doTranspose = itsInputNodes.size() > 0 && itsOutputNodes.size() > 0;

  for (unsigned node = 0, cell = 0, station = 0; node < nrNodes; node ++) {
    bool doInput  = std::find(itsInputNodes.begin(), itsInputNodes.end(), node) != itsInputNodes.end();
    bool doOutput = std::find(itsOutputNodes.begin(), itsOutputNodes.end(), node) != itsOutputNodes.end();
    TransportHolder *th = 0;

    if (doInput) {
      th = Connector::readTH(itsCS1PS, itsCS1PS->stationName(station)); 
    }

    itsWHs[node] = new WH_InputSection("InputSection", doInput, doTranspose, doOutput, itsCS1PS, th, doInput ? station : 0, doInput ? 1 : 0, doOutput ? nrOutputChannels : 0, itsInputNodes, itsOutputNodes);
    Step *step = new Step(itsWHs[node], "Step", false);
    step->runOnNode(node); 
    comp.addBlock(step);

    if (doInput) {
      itsDelayStub->connect(station, step->getInDataManager(0), 0);
      station ++;
    }

    if (doOutput) {
      DataManager      &dm = step->getOutDataManager(0);
      std::vector<int> channels(nrOutputChannels);

      for (unsigned core = 0; core < nrOutputChannels; core ++) {
	dm.setOutBuffer(core, false, itsCS1PS->useScatter() ? 8 : 3);
	itsOutputStub->connect(cell, core, dm, core);
	channels[core] = core;
      }
	
      dm.setOutRoundRobinPolicy(channels, itsCS1PS->getInt32("OLAP.BGLProc.maxConcurrentComm"));
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
