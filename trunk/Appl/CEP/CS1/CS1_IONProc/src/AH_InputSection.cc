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
#include <CS1_Interface/RSPTimeStamp.h>
#include <CEPFrame/Step.h>
#include <AH_InputSection.h>
#include <WH_InputSection.h>
#include <BGL_Personality.h>
#include <Connector.h>

//# Workholders

#include <Transport/TransportHolder.h>
#include <Transport/TH_MPI.h>

#include <algorithm>

namespace LOFAR {
namespace CS1 {

AH_InputSection::AH_InputSection() :
  itsCS1PS(0),
  itsDelayStub(0),
  itsWH(0)
{
}

AH_InputSection::~AH_InputSection()
{
  std::clog << "AH_InputSection::~AH_InputSection()" << std::endl;
  undefine();
}

void AH_InputSection::define(const LOFAR::KeyValueMap&) 
{
  LOG_TRACE_FLOW_STR("Start of AH_InputSection::define()");

  itsCS1PS = new CS1_Parset(&itsParamSet);
  itsCS1PS->adoptFile("OLAP.parset");

  TimeStamp::setMaxBlockId(itsCS1PS->sampleRate());

  // TODO: support multiple RSPs per station
#if 0
  itsInputNodes  = itsCS1PS->getUint32Vector("Input.InputNodes");
  itsOutputNodes = itsCS1PS->getUint32Vector("Input.OutputNodes");
  unsigned nrOutputChannels = itsCS1PS->nrOutputsPerInputNode();
#endif

  unsigned myPsetNumber	 = getBGLpersonality()->getPsetNum();
  unsigned stationNumber = itsCS1PS->inputPsetIndex(myPsetNumber);
  std::clog << "station " << stationNumber << " = " << itsCS1PS->stationName(stationNumber) << std::endl;
  TransportHolder *th = Connector::readTH(itsCS1PS, itsCS1PS->stationName(stationNumber)); //FIXME probably never deleted
  itsWH = new WH_InputSection("InputSection", stationNumber, itsCS1PS, th);
  Step *step = new Step(itsWH, "Step", false);
  step->runOnNode(0); 
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite
  comp.addBlock(step);

  itsDelayStub  = new Stub_Delay(true, itsCS1PS);
  itsDelayStub->connect(stationNumber, step->getInDataManager(0), 0);

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

void AH_InputSection::undefine()
{
  delete itsWH;		itsWH        = 0;
  delete itsDelayStub;	itsDelayStub = 0;
  delete itsCS1PS;	itsCS1PS     = 0;
}


} // namespace CS1
} // namespace LOFAR
