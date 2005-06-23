//#  WH_DelayControl.cc: 
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
#include <WH_DelayControl.h>
#include <TFC_Interface/DH_Sync.h>

namespace LOFAR{

WH_DelayControl::WH_DelayControl(const string& name,
				 const int    nRSPInputs)
  : WorkHolder(1, nRSPInputs, name, "WH_DelayControl")
{
  char str[128];
    for (int i = 0; i < itsNinputs; i++) {
      getDataManager().addInDataHolder(i, new DH_Sync(str)); 
  }
  for (int i = 0; i < itsNoutputs; i++) {
    snprintf(str, 128, "output_%d_of _%d", i, itsNoutputs);
    getDataManager().addOutDataHolder(i, new DH_Sync(str));
  }
}

WH_DelayControl::~WH_DelayControl() {
}

WorkHolder* WH_DelayControl::construct(const string& name, 
				       const int nRSPInputs)
{
  return new WH_DelayControl(name, nRSPInputs);
}

WH_DelayControl* WH_DelayControl::make(const string& name) {
  return new WH_DelayControl(name, itsNoutputs);
}

void WH_DelayControl::process() {
  DH_Sync* inp = (DH_Sync*)getDataManager().getInHolder(0);
  DH_Sync* outp = 0;
  for (int i = 0; i < itsNoutputs; i++) {
    outp = (DH_Sync*)getDataManager().getOutHolder(i);
    // To to: copy delays from input to output
  }
}
}
