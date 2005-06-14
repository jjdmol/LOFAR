//#  WH_SyncControl.cc: 
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//#
///////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <WH_SyncControl.h>
#include <TFC_Interface/DH_Sync.h>

namespace LOFAR{

WH_SyncControl::WH_SyncControl(const string& name
			       //			       const int&    noutputs
			       )
: WorkHolder(1, 1, name, "WH_SyncControl")
    //todo: correct nr of inputs and outputs; each RSPInput node should
    //      receive a sync/delay DH from the syncController.
{
  char str[128];
    for (int i = 0; i < itsNinputs; i++) {
    //    getDataManager().addInDataHolder(i, new DH_StationData(str, bufsize));
  }
  for (int i = 0; i < itsNoutputs; i++) {
    snprintf(str, 128, "output_%d_of _%d", i, itsNoutputs);
    getDataManager().addOutDataHolder(i, new DH_Sync(str));
  }
}

WH_SyncControl::~WH_SyncControl() {
}

WorkHolder* WH_SyncControl::construct(const string& name) {
  return new WH_SyncControl(name);
}

WH_SyncControl* WH_SyncControl::make(const string& name) {
  return new WH_SyncControl(name);
}

void WH_SyncControl::process() {
}
}
