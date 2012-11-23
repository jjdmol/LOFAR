//#  VHECRsettings.cc: one line description
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: VHECRsettings.cc 9881 2007-01-24 12:39:40Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <ApplCommon/lofar_datetime.h>
#include <Common/StringUtil.h>

#include "VHECR/VHECRsettings.h"

namespace LOFAR {
	namespace VHECR {

VHECRsettings::VHECRsettings()
{
  itsSettingsLoaded = false;
  // fill the Object with default or dummy values
  antennaSet        = "NONE";
  antennaField      = NULL;
  clockFreq         = 200;
  noCoincChann      = 50;
  coincidenceTime   = 1e-6;
  doDirectionFit    = 0;
  minElevation      = 0.;
  maxFitVariance    = 50.;

  LOG_DEBUG_STR("VHECR settings defined with default/dummy values");
}

VHECRsettings::VHECRsettings(ParameterSet* aParSet)
{
	itsSettingsLoaded = false;
	string prefix;

	antennaField = globalAntennaField();

	// analyse ParameterSet.
	// get Observation parameters
	prefix = aParSet->locateModule("Observation") + "Observation.";
	LOG_DEBUG_STR("'Observation' located at: " << prefix);

	clockFreq = aParSet->getInt32(prefix+"sampleClock");
	antennaSet = aParSet->getString(prefix+"antennaSet");

	RCUsetActive.reset();                           // clear RCUset by default.
	string  rcuString("x=" + expandArrayString(aParSet->getString(prefix+"receiverList")));
	ParameterSet    rcuParset;
	rcuParset.adoptBuffer(rcuString);
	vector<uint16> RCUnumbers(rcuParset.getUint16Vector("x"));
	if (RCUnumbers.empty()) {           // No receivers in the list?
		RCUsetActive.set();                     // assume all receivers
	} else {
		for (uint i = 0; i < RCUnumbers.size();i++) {
			RCUsetActive.set(RCUnumbers[i]);    // set mentioned receivers in all set
		}
	}

	// get VHECR parameters
	prefix = aParSet->locateModule("Observation") + "Observation.ObservationControl.StationControl.TBBControl.";
	LOG_DEBUG_STR("'TBB' located at: " << prefix);


	// look if the first VHECR setting is defined, asuming the rest is there also
	if (aParSet->isDefined(prefix+"NoCoincChann")) {
		LOG_DEBUG_STR("Reading parameterSet ");
		itsSettingsLoaded = true;

		noCoincChann    = aParSet->getInt32(prefix+"NoCoincChann");
		coincidenceTime = aParSet->getDouble(prefix+"CoincidenceTime", 1.01e-6);
		string dofit = aParSet->getString(prefix+"DoDirectionFit");
		if (dofit == "none") { doDirectionFit = 0; }
		else if (dofit == "simple") { doDirectionFit = 1; }
		else if (dofit == "fancy") { doDirectionFit = 2; }
		minElevation    = aParSet->getDouble(prefix+"MinElevation", 30.5);
		maxFitVariance  = aParSet->getDouble(prefix+"MaxFitVariance");
        
        
        string prefix = aParSet->locateModule("Observation") + "Observation.TBB.TBBsetting";
	    LOG_DEBUG_STR("'TBB' located at: " << prefix);

        int setNr = 0;
	    string setnr(formatString("[%d].", setNr));
	    
	    RCUsetSelected.reset();                         // clear RCUset by default.
	    while (aParSet->isDefined(prefix+setnr+"RCUs")) {
		    LOG_DEBUG_STR("Reading SetNr " << setNr);
		    string  rcuString("x=" + expandArrayString(aParSet->getString(prefix+setnr+"RCUs")));
		    ParameterSet    rcuParset;
		    rcuParset.adoptBuffer(rcuString);
		    vector<uint16> RCUnumbers(rcuParset.getUint16Vector("x"));
		    if (!RCUnumbers.empty()) {           // No receivers in the list?
			    for (uint i = 0; i < RCUnumbers.size(); i++) {
				    RCUsetSelected.set(RCUnumbers[i]);  // set mentioned receivers in all set
			    }
		    }
		    setNr++;
	        setnr = formatString("[%d]", setNr);
		}
        LOG_DEBUG_STR("active RCU set  " << RCUsetActive);
        LOG_DEBUG_STR("selected RCU set" << RCUsetSelected);
        RCUsetActive &= RCUsetSelected;
		//SubbandList = aParSet->getInt32Vector(prefix+"subbandList", vector<int32>(), true);
	}
	else {
		LOG_DEBUG_STR("VHECR settings NOT defined");
	}
}

//#
//# operator<<
//#
ostream& VHECRsettings::print(ostream&  os) const
{
	os << "requested RCUset : " << RCUsetSelected << endl;
	os << "active RCUset    : " << RCUsetActive << endl;
	
	os << "NoCoincChann     : " << noCoincChann << endl;
	os << "CoincidenceTime  : " << coincidenceTime << endl;
	os << "DoDirectionFit   : " << doDirectionFit << endl;
	os << "MinElevation     : " << minElevation << endl;
	os << "MaxFitVariance   : " << maxFitVariance << endl;
	
    os << "antennaSet       : " << antennaSet << endl;
    os << "antennaField     : " << antennaField << endl;
	os << endl;

	return (os);
}

	}
}
