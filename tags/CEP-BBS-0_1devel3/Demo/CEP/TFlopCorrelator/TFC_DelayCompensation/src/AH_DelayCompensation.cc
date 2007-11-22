//#  AH_DelayCompensation.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//
/////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <TFC_DelayCompensation/AH_DelayCompensation.h>
#include <TFC_DelayCompensation/WH_DelayControl.h>
#include <Common/lofar_iostream.h>
#include <APS/ParameterSet.h>
#include <CEPFrame/Step.h>
#include <TFC_Interface/Stub_Delay.h>
#include <TFC_Interface/Stub_PhaseCorr.h>

using namespace LOFAR;

AH_DelayCompensation::AH_DelayCompensation() 
  : itsDelayStub    (0),
    itsPhaseCorrStub(0)
{
}

AH_DelayCompensation::~AH_DelayCompensation() {
  this->undefine();
}

void AH_DelayCompensation::undefine() {
  delete itsDelayStub;
  delete itsPhaseCorrStub;
}  

void AH_DelayCompensation::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_DelayCompensation::define()");
  undefine();

  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level compisite

  int nStations = itsParamSet.getInt32("Data.NStations");
  int nChannels = itsParamSet.getInt32("Data.NChannels");
  int delay = itsParamSet.getInt32("Data.Delays.Station0_Station1");
  WH_DelayControl delayWH("DelayContr", nStations, nChannels, delay);
  Step delayStep(delayWH, "DelayContr");
  comp.addBlock(delayStep);

  // Connect to stub for input section
  itsDelayStub = new Stub_Delay(false, itsParamSet);
  for (int i=0; i<nStations; i++)
  {
    itsDelayStub->connect(i, delayStep.getOutDataManager(i), i);
  }

//   // Connect to stub for BGL proc
//   itsPhaseCorrStub = new Stub_PhaseCorr(false, itsParamSet);
//   itsPhaseCorrStub->connect(delayStep.getOutDataManager(nStations), nStations);

  LOG_TRACE_FLOW_STR("Finished define()");
}



void AH_DelayCompensation::prerun() {
  getComposite().preprocess();
}
    
void AH_DelayCompensation::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_DelayCompensation::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_DelayCompensation::run() "  );
}

void AH_DelayCompensation::dump() const {
  LOG_TRACE_FLOW_STR("AH_DelayCompensation::dump() not implemented"  );
}

void AH_DelayCompensation::quit() {

}


