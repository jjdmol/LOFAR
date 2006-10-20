//#  Observation.cc: one line description
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <APL/APLCommon/Observation.h>

namespace LOFAR {
  namespace APLCommon {

Observation::Observation() :
	name(),
	treeID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	subbands(),
	sampleClock(0)
{
}

Observation::Observation(ACC::APS::ParameterSet*		aParSet) :
	name(),
	treeID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	subbands(),
	sampleClock(0)
{
	// analyse ParameterSet.
	string prefix = ACC::APS::moduleName(aParSet->locateModule("Observation"));
	if (aParSet->isDefined(prefix+"name")) {
		name = aParSet->getString(prefix+"name");
	}
	if (aParSet->isDefined(prefix+"treeID")) {
		treeID = aParSet->getInt32(prefix+"treeID");
	}
	if (aParSet->isDefined(prefix+"startTime")) {
		startTime = to_time_t(time_from_string(aParSet->getString(prefix+"startTime")));
	}
	if (aParSet->isDefined(prefix+"stopTime")) {
		stopTime = to_time_t(time_from_string(aParSet->getString(prefix+"stopTime")));
	}
	if (aParSet->isDefined(prefix+"nyquistZone")) {
		nyquistZone = aParSet->getInt16(prefix+"nyquistZone");
	}
	if (aParSet->isDefined(prefix+"subbandList")) {
		subbands = aParSet->getInt16Vector(prefix+"subbandList");
	}
	if (aParSet->isDefined(prefix+"bandFilter")) {
		filter = aParSet->getString(prefix+"bandFilter");
	}
	if (aParSet->isDefined(prefix+"antennaArray")) {
		antennaArray = aParSet->getString(prefix+"antennaArray");
	}
	RCUset.reset();							// clear RCUset by default.
	if (aParSet->isDefined(prefix+"receiverList")) {
		vector<uint16> RCUnumbers(aParSet->getUint16Vector(prefix+"receiverList"));
		if (RCUnumbers.empty()) {			// No receivers in the list?
			RCUset.set();					// assume all receivers.
		}
		else {
			for (uint i = 0; i < RCUnumbers.size();i++) {
				RCUset.set(RCUnumbers[i]);	// set mentioned receivers
			}
		}
	}
	if (aParSet->isDefined(prefix+"sampleClock")) {
		sampleClock = aParSet->getUint32(prefix+"sampleClock");
	}
}


Observation::~Observation()
{
}



  } // namespace APLCommon
} // namespace LOFAR
