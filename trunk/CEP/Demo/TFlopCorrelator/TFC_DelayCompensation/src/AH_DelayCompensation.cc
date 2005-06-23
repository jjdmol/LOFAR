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
#include <Common/lofar_iostream.h>
#include <ACC/ParameterSet.h>
#include <CEPFrame/Step.h>

using namespace LOFAR;

AH_DelayCompensation::AH_DelayCompensation() 
{
}

AH_DelayCompensation::~AH_DelayCompensation() {
  this->undefine();
}

void AH_DelayCompensation::undefine() {
}  

void AH_DelayCompensation::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_DelayCompensation::define()");
  undefine();

  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp;
  setComposite(comp); // tell the AppllicationHolder this is the top-level compisite


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


