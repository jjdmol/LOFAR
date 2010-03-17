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
	TBBsetting()
{
}

TBBObservation::TBBObservation(ParameterSet*		aParSet)
{
	// analyse ParameterSet.
	string prefix = aParSet->locateModule("Observation") + "Observation.TBB.";
	LOG_DEBUG_STR("'TBB' located at: " << prefix);

	sTBBsetting tbbsetting;
	
	int setNr = 0;
	string setname(formatString("%sTBBsetting[%d]", prefix.c_str(),setNr));
	// look if the TBB settings are defined
	while (aParSet->isDefined(setname+".C0")) {
		LOG_DEBUG_STR("Reading parameterSet " << setNr);
		tbbsetting.c0           = aParSet->getInt16(setname+".C0");
		tbbsetting.c1           = aParSet->getInt16(setname+".C1");
		tbbsetting.c2           = aParSet->getInt16(setname+".C2");
		tbbsetting.c3           = aParSet->getInt16(setname+".C3");
		tbbsetting.triggerLevel = aParSet->getInt16(setname+".baselevel");
		tbbsetting.filter       = aParSet->getInt16(setname+".filter");
		tbbsetting.startLevel   = aParSet->getInt16(setname+".startlevel");
		tbbsetting.stopLevel    = aParSet->getInt16(setname+".stoplevel");
		tbbsetting.detectWindow = _windowNr(aParSet->getString(setname+".window"));
		
		// operatingMode from first observation set is used for all boards
		if (setNr == 0) {
			operatingMode = aParSet->getUint32(setname+".operatingMode");
		}
						
		tbbsetting.RCUset.reset();							// clear RCUset by default.
		string	rcuString("x=" + expandArrayString(
											aParSet->getString(setname+".RCUs")));
		ParameterSet	rcuParset;
		rcuParset.adoptBuffer(rcuString);
		vector<uint16> RCUnumbers(rcuParset.getUint16Vector("x"));
		if (RCUnumbers.empty()) {			// No receivers in the list?
			tbbsetting.RCUset.set();		// assume all receivers.
			allRCUset.set();						// assume all receivers
		} else {
			for (uint i = 0; i < RCUnumbers.size();i++) {
				tbbsetting.RCUset.set(RCUnumbers[i]);	// set mentioned receivers in this set
				allRCUset.set(RCUnumbers[i]);	// set mentioned receivers in all set
			}
		}
		// get rcuset
		TBBsetting.push_back(tbbsetting);
		setNr++;
		setname = formatString("%sTBBsetting[%d]", prefix.c_str(),setNr);
	}
	prefix = aParSet->locateModule("Observation") + "Observation.";
	int beam = 0;
	string	beamPrefix(prefix+formatString("Beam[%d].", beam));
	subbandList = aParSet->getInt32Vector(beamPrefix+"subbandList", vector<int32>(), true);
}

//
// _windowNr
//
uint16 TBBObservation::_windowNr(const string&		wdwName)
{
	static	char*		windowNames[] = {
		"16B", "64B", "256B", "1K", "4K", "16K", "64K", "" };

	uint32	idx = 0;
	while (windowNames[idx][0]) {
		if (!strcmp(wdwName.c_str() , windowNames[idx])) {
			return (idx);
		}
		idx++;
	}

	LOG_WARN_STR("detectWindow '" << wdwName << "' is unknown, assuming 64B");

	return (1);
}

//#
//# operator<<
//#
ostream& TBBObservation::print(ostream&	os) const
{
	int setNr = 0;
	os << "operatingMode  : " << operatingMode << endl;
	os << "RCUset         : " << allRCUset << endl;
	os << "subbandList    : ";
	std::vector<int32>::const_iterator sb_it;
	for (sb_it = subbandList.begin(); sb_it != subbandList.end(); sb_it++) {
		os << (*sb_it) << " ";
	}
	os << endl << endl;
	vector<sTBBsetting>::const_iterator it;
	for (it = TBBsetting.begin(); it != TBBsetting.end(); it++ ) {
		os << "Observation set: " << setNr << endl; 
		os << "Coeff-0        : " << (*it).c0 << endl;
		os << "Coeff-1        : " << (*it).c1 << endl;
		os << "Coeff-2        : " << (*it).c2 << endl;
		os << "Coeff-3        : " << (*it).c3 << endl;
		os << "TriggerLevel   : " << (*it).triggerLevel << endl;
		os << "Filter         : " << (*it).filter << endl;
		os << "StartLevel     : " << (*it).startLevel << endl;
		os << "StopLevel      : " << (*it).stopLevel << endl;
		os << "detectWindow   : " << (*it).detectWindow << endl << endl;
		setNr++;
	}
	return (os);
}
