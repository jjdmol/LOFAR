//#  WH_Dump.cc: Output workholder for BG Correlator application
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, swe@astron.nl
//#
//#
//#  $Id$


#include <stdio.h>           // for sprintf

// General includes
#include <Common/LofarLogger.h>
#include <Common/KeyValueMap.h>

// Application specific includes
#include <WH_Dump.h>
#include <DH_Vis.h>

using namespace LOFAR;


WH_Dump::WH_Dump(const string& name,
		 const int    elements, 
		 const int    channels) 
  : WorkHolder(1, 0, name, "WH_Dump"),
    itsNelements (elements),
    itsNchannels (channels)
{
  getDataManager().addInDataHolder(0, new DH_Vis("in_", 
						 itsNelements, itsNchannels));
}

WH_Dump::~WH_Dump() {

}

WorkHolder* WH_Dump::construct (const string& name, 
				const int    elements, 
				const int    channels) {
  return new WH_Dump(name, elements, channels);
}

WH_Dump* WH_Dump::make(const string& name) {

  return new WH_Dump(name, itsNelements, itsNchannels);
}

void WH_Dump::process() {

  // negative indices are invalid. This are results calculated from 
  // uninitialized values.
  cout << "COR [" << -2+itsIndex++ <<"]: "<< *((DH_Vis*)getDataManager().getInHolder(0))->getBuffer() << endl;
}

void WH_Dump::dump() {
}
		 
