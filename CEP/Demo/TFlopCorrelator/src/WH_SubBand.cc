//#  WH_SubBand.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <ACC/ParameterSet.h>
#include <WH_SubBand.h>

#include <TFlopCorrelator/DH_SubBand.h>
#include <TFlopCorrelator/DH_CorrCube.h>

//#include <fftw.h>

using namespace LOFAR;

WH_SubBand::WH_SubBand(const string& name,
		       const short   subBandID):
 WorkHolder( 1, 1, name, "WH_Correlator"),
 itsSBID   (subBandID)
{

   ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
   //ParameterCollection	myPC(myPS);
   itsNtaps     = myPS.getInt("WH_SubBand.taps");
   itsNcoeff    = itsNtaps; // aren't these the same anyway?
   itsFFTLen    = myPS.getInt("WH_SubBand.fftlen");
   itsCpF       = myPS.getInt("WH_SubBand.Corr_per_Filter");

   getDataManager().addInDataHolder(0, new DH_SubBand("input", itsSBID));
   for (int c=0; c<itsCpF; c++) {
     getDataManager().addOutDataHolder(0, new DH_CorrCube("in", itsSBID)); 
   }
   
   //todo: Add DH for filter coefficients;
   //      need functionalit like the CEPFrame setAutotrigger.


  // Initialize the delay line
   delayLine = new FilterType[2*itsNtaps]; 
   memset(delayLine,0,2*itsNtaps*sizeof(FilterType));
   delayPtr = delayLine;
   
   // Need input: filter coefficients
   coeffPtr = new FilterType[itsNcoeff];
   for (int j = 0; j < itsNcoeff; j++) {
     __real__ coeffPtr[j] = (j + 1);
   }
   
}

WH_SubBand::~WH_SubBand() {
}

WorkHolder* WH_SubBand::construct(const string& name,
				  const short   SBID) {
  return new WH_SubBand(name, SBID);
}

WH_SubBand* WH_SubBand::make(const string& name) {
  return new WH_SubBand(name, itsSBID);
}

void WH_SubBand::process() {
  
  acc = 0 + 0i;

  for (int i = 0; i < itsNtaps; i++) { 
    
    acc += coeffPtr[i] * delayLine[i];
    
  }


}

void WH_SubBand::dump() {
}

void WH_SubBand::adjustDelayPtr() { 
  for (int i = itsNtaps - 2; i >= 0; i--) {
    delayLine[i+1] = delayLine[i];
  }

}
