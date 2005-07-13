//#  WH_FIR.cc: 256 kHz polyphase filter
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APS/ParameterSet.h>
#include <WH_FIR.h>

#include <TFC_Interface/DH_FIR.h>
#include <TFC_Interface/DH_PPF.h>

using namespace LOFAR;

WH_FIR::WH_FIR(const string& name,
		       const short   subBandID):
 WorkHolder( 1, 1, name, "WH_Correlator"),
 itsSBID       (subBandID)
{

   ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
   itsNFilters   = myPS.getInt32("WH_FIR.filters");
   itsNtaps      = myPS.getInt32("WH_FIR.taps");
   itsNStations  = myPS.getInt32("WH_FIR.stations");
   itsNTimes     = myPS.getInt32("WH_FIR.times");
   itsNPol       = myPS.getInt32("WH_FIR.pols");
   itsFFTs       = myPS.getInt32("WH_FIR.FFTs");

   getDataManager().addInDataHolder(0, new DH_FIR("input", itsSBID));

   for (int c=0; c<itsFFTs; c++) {
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

WH_FIR::~WH_FIR() {
  for (int filter = 0; filter < itsNFilters; filter++) {
    delete filterData[filter].delayLine;
    delete filterData[filter].filterTaps;
  }
  delete filterData;
}

WorkHolder* WH_FIR::construct(const string& name,
				  const short   SBID) {
  return new WH_FIR(name, SBID);
}

WH_FIR* WH_FIR::make(const string& name) {
  return new WH_FIR(name, itsSBID);
}

void WH_FIR::preprocess() {
}

void WH_FIR::process() {
  FilterType accum;
  short      write_to;
  short      write_index;

  for (int filter = 0; filter < itsNFilters; filter++) {
    accum = makefcomplex(0,0);
    
    // get data from the input dataholder
//     filterData[ filter ].delayLine[ 0 ] = 
//       *(static_cast<DH_FIR*>(getDataManager().getInHolder(0))->getBuffer() + filter);
    memcpy(&filterData[filter].delayLine[0],
	   (static_cast<DH_FIR*>(getDataManager().getInHolder(0))->getBuffer() + filter),
	   sizeof(DH_FIR::BufferType));
    


    for (int tap = 0; tap < itsNtaps; tap++) {
      accum += filterData[ filter ].filterTaps[ tap ] * 
	filterData[ filter ].delayLine[ tap ];
    }
    
    // make sure to write the output to the correct DataHolder
    // this may turn out to be a hotspot, since we're writing small amounts of data
    static_cast<DH_PPF*>(getDataManager().getOutHolder( filter % (itsNFilters/itsFFTs) ))->setBufferElement( filter / (itsNFilters/itsFFTs) , accum );
    adjustDelayPtr(filterData[filter].delayLine);
  }

}


void WH_FIR::postprocess() {
}

void WH_FIR::dump() {
}

void WH_FIR::adjustDelayPtr(FilterType* dLine) { 
  // this is a very inefficient implementation. Should be optimized later on.
  for (int i = itsNtaps - 2; i >= 0; i--) {
    dLine[i+1] = dLine[i];
  }
}
