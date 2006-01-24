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
#include <TFC_Interface/DH_Delay.h>
#include <TFC_Interface/DH_PhaseCorr.h>

namespace LOFAR{

WH_DelayControl::WH_DelayControl(const string& name,
				 const int     nRSPInputs,
				 const int     nrChannels,
				 const int     delay)
  : WorkHolder    (0, nRSPInputs+1, name, "WH_DelayControl"),
    itsNrRSPInputs(nRSPInputs),
    itsNrChannels (nrChannels),
    itsDelay   (delay)
{
  char str[128];
  for (int i = 0; i < itsNrRSPInputs; i++) {
    snprintf(str, 128, "output_%d_of _%d", i, itsNrRSPInputs);
    getDataManager().addOutDataHolder(i, new DH_Delay(str, itsNrRSPInputs));
  }
  getDataManager().addOutDataHolder(itsNrRSPInputs, new DH_PhaseCorr("PhaseCorrOutput", itsNrChannels));
}

WH_DelayControl::~WH_DelayControl() {
}

WorkHolder* WH_DelayControl::construct(const string& name, 
				       const int nRSPInputs,
				       const int nrChannels,
				       const int delay)
{
  return new WH_DelayControl(name, nRSPInputs, nrChannels, delay);
}

WH_DelayControl* WH_DelayControl::make(const string& name) {
  return new WH_DelayControl(name, itsNrRSPInputs, itsNrChannels, itsDelay);
}

void WH_DelayControl::process() {
  DH_Delay* outp = 0;
  // Set delay output
  for (int j = 0; j < itsNrRSPInputs; j++) 
  {    
    outp = (DH_Delay*)getDataManager().getOutHolder(j);
    for (int i=0; i < itsNrRSPInputs; i++)
    {
      outp->setDelay(i, i*itsDelay);
    }
  }

  // Set phase correction output
  DH_PhaseCorr* corrOutp;
  corrOutp = (DH_PhaseCorr*)getDataManager().getOutHolder(itsNrRSPInputs);
  for (int ch=0; ch<itsNrChannels; ch++)
  {
    corrOutp->setPhaseCorr(ch, makefcomplex(1,-1));
  }

}

}
