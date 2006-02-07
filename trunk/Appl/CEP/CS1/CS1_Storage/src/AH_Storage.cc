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

#include <APS/ParameterSet.h>
#include <CS1_Storage/AH_Storage.h>
#include <Transport/TH_MPI.h>
#include <CS1_Interface/Stub_BGL_Visibilities.h>
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CEPFrame/Step.h>

using namespace LOFAR;

AH_Storage::AH_Storage() 
  : itsStub (0)
{
}

AH_Storage::~AH_Storage() {
}

void AH_Storage::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_Storage::define()");

  LOG_TRACE_FLOW_STR("Create the top-level composite");
  Composite comp(0, 0, "topComposite");
  setComposite(comp); // tell the ApplicationHolder this is the top-level composite

  itsStub = new Stub_BGL_Visibilities(false, itsParamSet);

  int nrSubbands = itsParamSet.getInt32("Data.NSubbands");

  for (int subb=0; subb< nrSubbands; subb++)
  {
    WH_SubbandWriter wh("storage1", subb, itsParamSet);
    Step step(wh);
    comp.addBlock(step);

    step.runOnNode(subb);    // Note: only 1 subband is written on a node

    // Connect to BG output
    for (int nr=0; nr<step.getNrInputs(); nr++)
    {
      itsStub->connect(subb, nr, step.getInDataManager(nr), nr);
    }

  }
  LOG_TRACE_FLOW_STR("Finished define()");

#ifdef HAVE_MPI
  ASSERTSTR (TH_MPI::getNumberOfNodes() == 1, "CS1_Storage should be started on just one node");
#endif

}

void AH_Storage::prerun() {
  getComposite().preprocess();
}
    
void AH_Storage::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_Storage::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    cout<<"run "<<i+1<<" of "<<steps<<endl;
    getComposite().process();
  }
  LOG_TRACE_FLOW_STR("Finished AH_Storage::run() "  );
}

void AH_Storage::dump() const {
  LOG_TRACE_FLOW_STR("AH_Storage::dump() not implemented"  );
}

void AH_Storage::quit() {

}


