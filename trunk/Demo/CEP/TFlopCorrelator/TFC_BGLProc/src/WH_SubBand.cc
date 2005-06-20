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

#include <TFC_Interface/DH_SubBand.h>
#include <TFC_Interface/DH_PPF.h>

using namespace LOFAR;

WH_SubBand::WH_SubBand(const string& name,
		       const short   subBandID):
 WorkHolder( 1, 1, name, "WH_Correlator"),
 itsSBID       (subBandID)
{

   ACC::ParameterSet  myPS("TFlopCorrelator.cfg");
   //ParameterCollection	myPC(myPS);
   itsNFilters   = myPS.getInt("WH_SubBand.filters");
   itsNtaps      = myPS.getInt("WH_SubBand.taps");
   itsNStations  = myPS.getInt("WH_SubBand.stations");
   itsNTimes     = myPS.getInt("WH_SubBand.times");
   itsNPol       = myPS.getInt("WH_SubBand.pols");
   itsFFTs       = myPS.getInt("WH_SubBand.FFTs");

   getDataManager().addInDataHolder(0, new DH_SubBand("input", itsSBID));

   for (int c=0; c<itsFFTs; c++) {
     // this should probably be two DataHolders
     getDataManager().addOutDataHolder(c, new DH_PPF("output", itsSBID)); 
   }
   
   filterData = new filterBox[itsNFilters];
   
   for (int filter = 0; filter<itsNFilters; filter++) {
     filterData[filter].filter_id = filter;
     filterData[filter].delayLine = new FilterType[itsNtaps];
     filterData[filter].filterTaps = new float[itsNtaps];

     memset(filterData[filter].delayLine, 0, itsNtaps*sizeof(FilterType));
     memset(filterData[filter].filterTaps, 0, itsNtaps*sizeof(float));
   }

}

WH_SubBand::~WH_SubBand() {
  for (int filter = 0; filter < itsNFilters; filter++) {
    delete filterData[filter].delayLine;
    delete filterData[filter].filterTaps;
  }
  delete filterData;
}

WorkHolder* WH_SubBand::construct(const string& name,
				  const short   SBID) {
  return new WH_SubBand(name, SBID);
}

WH_SubBand* WH_SubBand::make(const string& name) {
  return new WH_SubBand(name, itsSBID);
}

void WH_SubBand::preprocess() {
}

void WH_SubBand::process() {
  FilterType accum;

  for (int filter = 0; filter < itsNFilters; filter++) {
    accum = 0.0;
    
    for (int tap = 0; tap < itsNtaps; tap++) {
      accum += filterData[ filter ].filterTaps[ tap ] * 
	filterData[ filter ].delayLine[ tap ];
    }
    
    static_cast<DH_PPF*>(getDataManager().getOutHolder(0))->setBufferElement(filter, accum);
    adjustDelayPtr(filterData[filter].delayLine);
  }

}


void WH_SubBand::postprocess() {
}

void WH_SubBand::dump() {
}

void WH_SubBand::adjustDelayPtr(FilterType* dLine) { 
  // this is a very inefficient implementation. Should be optimized later on.
  for (int i = itsNtaps - 2; i >= 0; i--) {
    dLine[i+1] = dLine[i];
  }
}
