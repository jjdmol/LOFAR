//#  AH_Storage.cc: 
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
#include <Common/lofar_iostream.h>

#include <ACC/ParameterSet.h>

#include <AH_Storage.h>

// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
// Workholders
#include <tinyCEP/WorkHolder.h>
#include <CEPFrame/WH_Empty.h>
#include <WH_RSPInput.h>
#include <WH_Transpose.h>
#include <SB_Stub.h>

// DataHolders
#include <DH_SubBand.h>

using namespace LOFAR;

AH_Storage::AH_Storage() 
{
}

AH_Storage::~AH_Storage() {
  this->undefine();
}

void AH_Storage::undefine() {
  vector<WorkHolder*>::iterator it = itsWHs.begin();
  for (; it!=itsWHs.end(); it++) {
    delete *it;
  }
  itsWHs.clear();
}  

void AH_Storage::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_Storage::define()");
  undefine();

  int lowestFreeNode = 0;
  itsNSBF  = itsParamSet.getInt("NSBF");  // number of SubBand filters in the application
  
  
  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp;
  setComposite(comp); // tell the AppllicationHolder this is the top-level compisite

  LOG_TRACE_FLOW_STR("Finished define()");
}



void AH_Storage::prerun() {
  getComposite().preprocess();
}
    
void AH_Storage::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_Storage::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_Storage::run() "  );
}

void AH_Storage::dump() const {
  LOG_TRACE_FLOW_STR("AH_Storage::dump() not implemented"  );
}

void AH_Storage::quit() {

}


