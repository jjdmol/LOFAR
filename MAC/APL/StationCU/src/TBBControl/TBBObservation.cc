//#  TBBObservation.cc: one line description
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
//#  $Id: TBBObservation.cc 9881 2007-01-24 12:39:40Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Common/StringUtil.h>

#include "TBBObservation.h"

using namespace LOFAR;
using namespace StationCU;

TBBObservation::TBBObservation() :
	TbbSettings()
{
}

TBBObservation::TBBObservation(ParameterSet*		aParSet)
{
	itsSettingsLoaded = false;
	// analyse ParameterSet.
	string prefix = aParSet->locateModule("Observation") + "Observation.TBB.TBBsetting.";
	LOG_DEBUG_STR("'TBB' located at: " << prefix);

	// look if the TBB settings are defined
	if (aParSet->isDefined(prefix+"F0C0")) {
		LOG_DEBUG_STR("Reading parameterSet ");
		itsSettingsLoaded = true;

		// new settings for two filterbanks
		TbbSettings.filter0[0]    = aParSet->getInt16(prefix+"F0C0");
		TbbSettings.filter0[1]    = aParSet->getInt16(prefix+"F0C1");
		TbbSettings.filter0[2]    = aParSet->getInt16(prefix+"F0C2");
		TbbSettings.filter0[3]    = aParSet->getInt16(prefix+"F0C3");
		TbbSettings.filter1[0]    = aParSet->getInt16(prefix+"F1C0");
		TbbSettings.filter1[1]    = aParSet->getInt16(prefix+"F1C1");
		TbbSettings.filter1[2]    = aParSet->getInt16(prefix+"F1C2");
		TbbSettings.filter1[3]    = aParSet->getInt16(prefix+"F1C3");
									 
		TbbSettings.triggerLevel  = aParSet->getInt16(prefix+"baselevel");
		TbbSettings.filter        = aParSet->getInt16(prefix+"filter");
		TbbSettings.startLevel    = aParSet->getInt16(prefix+"startlevel");
		TbbSettings.stopLevel     = aParSet->getInt16(prefix+"stoplevel");
		TbbSettings.detectWindow  = _windowNr(aParSet->getString(prefix+"window"));
		TbbSettings.triggerMode   = aParSet->getUint16(prefix+"triggerMode");
		TbbSettings.operatingMode = aParSet->getUint32(prefix+"operatingMode");

		RCUset.reset();							// clear RCUset by default.
		string	rcuString("x=" + expandArrayString(aParSet->getString(prefix+"RCUs")));
		ParameterSet	rcuParset;
		rcuParset.adoptBuffer(rcuString);
		vector<uint16> RCUnumbers(rcuParset.getUint16Vector("x"));
		if (RCUnumbers.empty()) {			// No receivers in the list?
			RCUset.set();						// assume all receivers
		} else {
			for (uint i = 0; i < RCUnumbers.size();i++) {
				RCUset.set(RCUnumbers[i]);	// set mentioned receivers in all set
			}
		}
		
		SubbandList = aParSet->getInt32Vector(prefix+"subbandList", vector<int32>(), true); 
	}
	else {
		LOG_DEBUG_STR("TBB parameterSet NOT defined");
	}
}

//
// _windowNr
//
uint16 TBBObservation::_windowNr(const string&		wdwName)
{
	static	char*		windowNames[] = {
		"4K", "16K", "64K", "256K", "1M", "4M", "16M", "64M", "256M", "" };

	uint32	idx = 0;
	while (windowNames[idx][0]) {
		if (!strcmp(wdwName.c_str() , windowNames[idx])) {
			return (idx);
		}
		idx++;
	}

	LOG_WARN_STR("detectWindow '" << wdwName << "' is unknown, assuming 1M");

	return (4);
}

//#
//# operator<<
//#
ostream& TBBObservation::print(ostream&	os) const
{
	os << "RCUset           : " << RCUset << endl;
	os << "subbandList      : ";
	std::vector<int32>::const_iterator sb_it;
	for (sb_it = SubbandList.begin(); sb_it != SubbandList.end(); sb_it++) {
		os << (*sb_it) << " ";
	}
	os << endl;
	os << "Filter-0 coeff-0 : " << TbbSettings.filter0[0] << endl;
	os << "Filter-0 coeff-1 : " << TbbSettings.filter0[1] << endl;
	os << "Filter-0 coeff-2 : " << TbbSettings.filter0[2] << endl;
	os << "Filter-0 coeff-3 : " << TbbSettings.filter0[3] << endl;
	os << "Filter-1 coeff-0 : " << TbbSettings.filter1[0] << endl;
	os << "Filter-1 coeff-1 : " << TbbSettings.filter1[1] << endl;
	os << "Filter-1 coeff-2 : " << TbbSettings.filter1[2] << endl;
	os << "Filter-1 coeff-3 : " << TbbSettings.filter1[3] << endl;
	os << "TriggerLevel     : " << TbbSettings.triggerLevel << endl;
	os << "Filter           : " << TbbSettings.filter << endl;
	os << "StartLevel       : " << TbbSettings.startLevel << endl;
	os << "StopLevel        : " << TbbSettings.stopLevel << endl;
	os << "detectWindow     : " << TbbSettings.detectWindow << endl;
	os << "triggerMode      : " << TbbSettings.triggerMode << endl;
	os << "operatingMode    : " << TbbSettings.operatingMode << endl << endl;
	return (os);
}
