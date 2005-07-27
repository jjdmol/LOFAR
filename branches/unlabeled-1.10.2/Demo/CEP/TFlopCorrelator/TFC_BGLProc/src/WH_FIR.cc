//#  WH_FIR.cc: FIR part of 256 kHz polyphase filter
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

WH_FIR::WH_FIR(const string& name, const short subBandID):
 WorkHolder( 1, 2, name, "WH_Correlator"),
 itsSBID       (subBandID),
 itsNFilters   (0),
 itsNtaps      (0),
 itsNStations  (0),
 itsNTimes     (0),
 itsNPol       (0),
 itsFFTs       (0)
{

  ACC::APS::ParameterSet  myPS("TFlopCorrelator.cfg");
  itsNFilterBanks   = myPS.getInt32("WH_FIR.filters");
  itsNTaps          = myPS.getInt32("WH_FIR.taps");
  itsNStations      = myPS.getInt32("WH_FIR.stations");
  itsNTimes         = myPS.getInt32("WH_FIR.times");
  itsNPol           = myPS.getInt32("WH_FIR.pols");
  itsFFTs           = myPS.getInt32("WH_FIR.FFTs");
  
  DBGASSERTSTR(itsNTimes%itsNFilters == 0,"times should be multiple of FIR banks.");
  itsOutTime = itsNTimes%itsNFilters;

  getDataManager().addInDataHolder(0, new DH_FIR("input", itsSBID, myPS));
  
  for (int c=0; c<itsFFTs; c++) {
    getDataManager().addOutDataHolder(c, new DH_PPF("output", itsSBID)); 
  }
  
  itsDelayLine = new DH_FIR::BufferType[itsNtations][itsNFilterBanks][itsNTaps];
  itsCoeff     = new float[ItsNFilterBanks][itsNTaps];
  itsBankState = new short[itsNStations][itsNFilterBanks];
  //OLD  filterData = new filterBox[itsNFilters];
  
  memset(itsDelayLine, 0, itsNStations*itsNFilterBanks*itsNtaps*sizeof(DH_FIR::BufferType));
  memset(itsCoeff    , 1,              itsNFilterBanks*itsNtaps*sizeof(float));
  memset(itsBankState, 0,              itsNFilterBanks*itsNStations*sizeof(short));
  
  //OLD for (int filter = 0; filter < itsNFilters; filter++) {
  //OLD    filterData[filter].filter_id = filter;
  //OLD    filterData[filter].delayLine = new FilterType[itsNtaps];
  //OLD    filterData[filter].filterTaps = new float[itsNtaps];
  //OLD    
  //OLD    memset(filterData[filter].delayLine, 0, itsNtaps*sizeof(FilterType));
  //OLD    memset(filterData[filter].filterTaps, 1, itsNtaps*sizeof(float));
  //OLD  }
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
  
  for (int station=0; station<itsNStations; station++) {
    for (int outtime=0; outtime<itsOutTimes; outtime++) {
      // within this loop we have to get itsNFilterBanks samples from the input;
      // each filter bank receives one time sample and produces one new output 
      // (weighted sum over all elements in the delay line)
      for (int filter = 0; filter < itsNFilterBanks; filter++) {
	
	outDH->getBufferElement[][][filter] = FIR(inDH->getBufferElement[][][],
						  station,
						  filter); 
	
	//OLD accum = makefcomplex(0,0);
	//OLD get data from the input dataholder
	//OLD     filterData[ filter ].delayLine[ 0 ] = 
	//OLD       *(static_cast<DH_FIR*>(getDataManager().getInHolder(0))->getBuffer() + filter);
	//OLD memcpy(&filterData[filter].delayLine[0],
	//OLD       (static_cast<DH_FIR*>(getDataManager().getInHolder(0))->getBuffer() + filter),
	//OLD       sizeof(DH_FIR::BufferType));
	//OLD
	//OLD	for (int tap = 0; tap < itsNtaps; tap++) {
	//OLD	  accum += filterData[ filter ].filterTaps[ tap ] * 
	//OLD	    filterData[ filter ].delayLine[ tap ];
	//OLD	}
	//OLD
	//OLD	// make sure to write the output to the correct DataHolder
	//OLD	// this may turn out to be a hotspot, since we're writing small amounts of data
	//OLD	static_cast<DH_PPF*>(getDataManager().getOutHolder( filter / (itsNFilters/itsFFTs) ))->setBufferElement( filter % (itsNFilters/itsFFTs) , accum );
	//OLD	adjustDelayPtr(filterData[filter].delayLine);
	//OLD

      }
    } // loop outtime
  } // loop over stations
}


void WH_FIR::postprocess() {
}

void WH_FIR::dump() const{
}


DH_FIR::BufferType WH_FIR::FIR_basic(DH_FIR::BufferType &sample,
		       short station,
		       short bank) {
    DH_FIR::BufferType accum;
    
    /* store input at the beginning of the delay line */
    itsDelayLine[station][bank][0] = sample;

    /* calc FIR */
    accum = 0;
    for (int tap = 0; tap < itsNtaps; tap++) {
        accum += itsCoeff[bank][tap] * itsDelayLine[station][bank][tap];
    }

    /* shift delay line */
    for (int tap = itsNtaps - 2; tap >= 0; tap--) {
        itsDelayLine[station][bank][tap + 1] = itsDelayLine[station][bank][tap];
    }

    return accum;
}

DH_FIR::BufferType WH_FIR::FIR_circular(DH_FIR::BufferType &sample,
				short station,
				short bank) {
  DH_FIR::BufferType accum;

  int state = itsBankState[station][bank];               /* copy the filter's state to a local */
  
  /* store input at the beginning of the delay line */
  itsDelayLine[station][bank][state] = sample;
  if (++state >= itsNtaps) {         /* incr state and check for wrap */
    state = 0;
  }
  
    /* calc FIR and shift data */
  accum = 0;
  for (int ii = itsNtaps - 1; ii >= 0; ii--) {
    accum += itsCopeff[bank][ii] * itsDelayLine[station][bank][state];
    if (++state >= ntaps) {     /* incr state and check for wrap */
      state = 0;
    }
  }
  itsBankState[station][bank] = state;               /* remember new state */

  return accum;
}

//OLD void WH_FIR::adjustDelayPtr(FilterType* dLine) { 
//OLD   // this is a very inefficient implementation. Should be optimized later on.
//OLD  for (int i = itsNtaps - 2; i >= 0; i--) {
//OLD    dLine[i+1] = dLine[i];
//OLD  }

}
