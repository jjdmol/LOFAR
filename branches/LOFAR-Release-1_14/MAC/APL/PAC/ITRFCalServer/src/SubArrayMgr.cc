//#  -*- mode: c++ -*-
//#  SubArrayMgr.cc: implementation of the SubArray class
//#
//#  Copyright (C) 2002-2004
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
//#  $Id: SubArrayMgr.cc 12256 2008-11-26 10:54:14Z overeem $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <APL/RTCCommon/Timestamp.h>
#include <APL/ICAL_Protocol/ICAL_Protocol.ph>
#include "SubArrayMgr.h"

//using namespace std;
using namespace blitz;

namespace LOFAR {
  using namespace RTC;
  using namespace GCF::TM;
  namespace ICAL {

// forward declaration
class CalibrationInterface;
    
//
// SubArrayMgr()
//
SubArrayMgr::SubArrayMgr()
{}

//
// ~SubArrayMgr()
//
SubArrayMgr::~SubArrayMgr()
{
	// delete subarrays from itsNewArrays map and clear map
	SubArrayMap::const_iterator		iter = itsNewArrays.begin();
	SubArrayMap::const_iterator		end  = itsNewArrays.end();
	while (iter != end) {
		if (iter->second) {
			delete iter->second;
		}
		++iter;
	}
	itsNewArrays.clear();

	// delete subarrays from itsActiveArrays map and clear map
	iter = itsActiveArrays.begin();
	end  = itsActiveArrays.end();
	while (iter != end) {
		if (iter->second) {
			delete iter->second;
		}
		++iter;
	}
	itsActiveArrays.clear();

	// clear itsDeadArrays list, subarrays have already been delete
	// because they were also in the itsActiveArrays map
	itsDeadArrays.clear();
}

//
// getRCUs(rcumode)
//
// Get a RCU mask of all RCU's that are used in the given rcumode.
// @param rcumode number 1..7 being one of the LOFAR receiver modes
// @return An bitmask containing all RCU's that are active for the rcumode.
RCUmask_t SubArrayMgr::getRCUs(uint	rcumode) const
{
	// SubArrayMap : map<string, SubArray*>
	SubArrayMap::const_iterator		iter = itsActiveArrays.begin();
	SubArrayMap::const_iterator		end  = itsActiveArrays.end();
	RCUmask_t	rcumask;
	while (iter != end) {
		if (iter->second->usesRCUmode(rcumode)) {
			rcumask |= iter->second->RCUMask(rcumode);
		}
		++iter;
	}
	return (rcumask);
}


// Send the given gains to all clients that use the given rcumode.
// @param rcumode number 1..7 being one of the LOFAR receiver modes
// @param gains an AntennaGain class containing the calibrated gains
void SubArrayMgr::publishGains(uint rcumode, const AntennaGains& theGains)
{
	// SubArrayMap : map<string, SubArray*>
	SubArrayMap::const_iterator		iter = itsActiveArrays.begin();
	SubArrayMap::const_iterator		end  = itsActiveArrays.end();
	while (iter != end) {
		if (iter->second->usesRCUmode(rcumode)) {
			ICALUpdateEvent	update;
			update.name			= iter->first;
			update.timestamp	= Timestamp::now();
			update.status		= ICAL_SUCCESS;
			update.gains		= theGains;
			LOG_INFO_STR("Sending new gains for rcumode " << rcumode << " of subarray " << iter->first);
			GCFPortInterface*	clientPort((itsPortMap.find(iter->first))->second);
			clientPort->send(update);
		}
		++iter;
	}
}


//
// scheduleAdd(SubArray*)
//
void SubArrayMgr::scheduleAdd(SubArray* array, GCFPortInterface*	port)
{
	ASSERTSTR(array && port, "Cannot add NULL subarray or subarray not connected to a port");

	LOG_DEBUG_STR("SCHEDULE_ADD:" << array->name());
	itsNewArrays[array->name()] = array;
	itsPortMap  [array->name()] = port;
}

//
// scheduleRemove(name)
//
bool SubArrayMgr::scheduleRemove(const string& name)
{
	LOG_DEBUG_STR("SCHEDULE_REMOVE:" << name);
	// find in itsNewArrays
	SubArrayMap::iterator iter = itsNewArrays.find(name);
	// if found then remove
	if (iter != itsNewArrays.end()) {
		delete iter->second;
		itsNewArrays.erase(iter);
		itsPortMap.erase(name);
		itsSubscriptionMap.erase(name);
		return (true);
	}

	// if not found in itsNewArrays, try to find in itsActiveArrays
	iter = itsActiveArrays.find(name);

	// if found then copy it to itsDeadArrays
	if (iter != itsActiveArrays.end()) {
		LOG_DEBUG_STR("scheduleRemove:Adding to deadlist " << name);
		itsDeadArrays.push_back(iter->second);
		return (true);
	}

	return (false);
}

//
// scheduleRemove(SubArray*)
//
bool SubArrayMgr::scheduleRemove(SubArray*& subarray)
{
	return (subarray ? scheduleRemove(subarray->name()) : false);
}

//
// activateArrays()
//
void SubArrayMgr::activateArrays()
{
	LOG_DEBUG("activateArrays()");

	// New subarrays are listed in itsNewArrays.
	// This method moves these subarrays to the itsActiveArrays map.
	SubArrayMap::const_iterator	iter = itsNewArrays.begin();
	SubArrayMap::const_iterator	end  = itsNewArrays.end();
	while (iter != end) {
		LOG_DEBUG_STR("activateArrays:Adding subarray " << (*iter).first);
		itsActiveArrays[(*iter).first] = (*iter).second; // add to itsActiveArrays
		++iter;
	}
	itsNewArrays.clear(); // clear itsNewArrays
}

//
// removeDeadArrays()
//
void SubArrayMgr::removeDeadArrays()
{
	// Subarrays that should be removed are listed in itsDeadArrays.
	// This method deletes these subarrays and removes them from the itsActiveArrays.
	// The itsDeadArrays list is cleared when done.
	SubArrayMap::iterator	activeIter;
	list<SubArray*>::const_iterator deadIter = itsDeadArrays.begin();
	list<SubArray*>::const_iterator deadEnd  = itsDeadArrays.end();
	while (deadIter != deadEnd) {
		/* Remove from the itsActiveArrays map*/
		activeIter = itsActiveArrays.find((*deadIter)->name());
		if (activeIter != itsActiveArrays.end()) {
			LOG_DEBUG_STR("removeDeadArrays:Removing subarray " << (*deadIter)->name());
			itsActiveArrays.erase(activeIter);
			itsPortMap.erase((*deadIter)->name());
		} else {
			LOG_FATAL_STR("trying to remove non-existing subarray " << (*deadIter)->name());
		}

		delete (*deadIter);
		++deadIter;
	}
	itsDeadArrays.clear();
}

//
// getByName(name)
//
SubArray* SubArrayMgr::getByName(const string& name)
{
	// find in itsNewArrays
	SubArrayMap::const_iterator it = itsNewArrays.find(name);

	if (it != itsNewArrays.end()) {
		return (*it).second;
	}

	// if not found in itsNewArrays, try to find in itsActiveArrays
	it = itsActiveArrays.find(name);

	if (it != itsActiveArrays.end()) {
		return (*it).second;
	}
	return 0;
}

//
// getSubArrays(optionalName)
//
SubArrayMap	SubArrayMgr::getSubArrays(const string&	optionalName) {
	// NOTE
	//
	// the administration of the subarrays is unfortunately not very simple.
	// when a new beam is started the beam is add to the itsNewArrays.
	// it stays there until the collection of the current ACM is finished
	// (worst case 511 seconds). When the ACM buffers are swapped the new
	// subarrays are added to the actual list.
	// The removal of stopped beams goes in the same way, when a beam is stopped
	// the beam is added to the itsDeadArrays but stays active in the calibration
	// cycle. At the swap of the ACM buffers the subarrays whos name is in the
	// dead-list are remove from the active m_subarrays.

	SubArrayMap	answer;

	if (optionalName.empty()) {         // no name specified?
		LOG_DEBUG("getSubArray: delivering all subarrays");
		// copy whole new array
		answer = itsNewArrays;
		// add active array
		SubArrayMap::const_iterator		iter = itsActiveArrays.begin();
		SubArrayMap::const_iterator		end  = itsActiveArrays.end();
		while (iter != end) {
			answer[iter->first] = iter->second;
			++iter;
		}
	}
	else {
		// get subarray by name
		SubArrayMap::const_iterator		subarray;
		if (((subarray = itsActiveArrays.find(optionalName)) != itsActiveArrays.end()) || 
			((subarray = itsNewArrays.find(optionalName)) != itsNewArrays.end()))  {
			answer[optionalName] = subarray->second;
		}
		else {
			LOG_DEBUG_STR("getSubArray: Subarray " << optionalName << " not found");
		}
	}

	// substract deadlist from constructed answer.
	list<SubArray*>::const_iterator		iter = itsDeadArrays.begin();
	list<SubArray*>::const_iterator		end  = itsDeadArrays.end();
	while (iter != end) {
		// try to find each deadlist-name in the answer
		SubArrayMap::const_iterator	saIter = answer.find((*iter)->name());
		if (saIter != answer.end()) {
			LOG_DEBUG_STR("SubArray " << (*iter)->name() << " on deadlist");
			answer.erase((*iter)->name());
		}
		++iter;
	}

	return (answer);
}

//
// addSubscription(name, port)
//
void SubArrayMgr::addSubscription(const string& subArrayName, GCFPortInterface*	port)
{
	ASSERTSTR(!subArrayName.empty(), "SubArray name must be sprecified");
	ASSERTSTR(port, "port must be sprecified");
	
	itsSubscriptionMap[subArrayName] = port;
}

//
// removeSubscription(name)
//
bool SubArrayMgr::removeSubscription(const string& subArrayName)
{
	ASSERTSTR(!subArrayName.empty(), "SubArray name must be sprecified");

//TODO return false when not found
	itsSubscriptionMap.erase(subArrayName);
	return (true);
}


  } // namespace ICAL
} // namespace LOFAR
