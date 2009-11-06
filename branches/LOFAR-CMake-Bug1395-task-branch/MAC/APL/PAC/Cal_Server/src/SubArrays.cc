//#  -*- mode: c++ -*-
//#  SubArrays.cc: implementation of the SubArray class
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
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/CAL_Protocol/ACC.h>
#include "SubArrays.h"

//using namespace std;
using namespace blitz;

namespace LOFAR {
  namespace CAL {

// forward declaration
class CalibrationInterface;
    
//
// SubArrays()
//
SubArrays::SubArrays()
{}

//
// ~SubArrays()
//
SubArrays::~SubArrays()
{
	// delete subarrays from m_new_arrays map and clear map
	for (SubArrayMap::const_iterator it = m_new_arrays.begin();
										it != m_new_arrays.end(); ++it) {
		if ((*it).second) {
			delete (*it).second;
		}
	}
	m_new_arrays.clear();

	// delete subarrays from m_arrays map and clear map
	for (SubArrayMap::const_iterator it = m_arrays.begin();
								it != m_arrays.end(); ++it) {
		if ((*it).second) {
			delete (*it).second;
		}
	}
	m_arrays.clear();

	// clear m_dead_arrays list, subarrays have already been delete
	// because they were also in the m_arrays map
	m_dead_arrays.clear();
}

//
// schedule_add(SubArray*)
//
void SubArrays::schedule_add(SubArray* array)
{
	LOG_DEBUG_STR("SCHEDULE_ADD:" << array->getName());
	if (array) {
		m_new_arrays[array->getName()] = array;
	}
}

//
// schedule_remove(name)
//
bool SubArrays::schedule_remove(const string& name)
{
	LOG_DEBUG_STR("SCHEDULE_REMOVE:" << name);
	// find in m_new_arrays
	SubArrayMap::iterator it = m_new_arrays.find(name);

	// if found then remove
	if (it != m_new_arrays.end()) {
		delete (*it).second;
		m_new_arrays.erase(it);
		return true;
	}

	// if not found in m_new_arrays, try to find in m_arrays
	it = m_arrays.find(name);

	// if found then move to m_dead_arrays
	if (it != m_arrays.end()) {
		LOG_DEBUG_STR("schedule_remove:Adding to deadlist " << name);
		m_dead_arrays.push_back((*it).second);
		return true;
	}

	return false;
}

//
// schedule_remove(SubArray*)
//
bool SubArrays::schedule_remove(SubArray*& subarray)
{
	return (subarray ? schedule_remove(subarray->getName()) : false);
}

//
// creator()
//
void SubArrays::creator()
{
	LOG_DEBUG("CREATOR()");

	/**
	* New subarrays are listed in m_new_arrays.
	* This method moves these subarrays to the m_arrays map.
	*/
	for (SubArrayMap::const_iterator it = m_new_arrays.begin(); it != m_new_arrays.end(); ++it) {
		LOG_DEBUG_STR("creator:Adding subarray " << (*it).second->getName());
		m_arrays[(*it).second->getName()] = (*it).second; // add to m_arrays
	}
	m_new_arrays.clear(); // clear m_new_array
}

//
// undertaker()
//
void SubArrays::undertaker()
{
	/**
	* Subarrays that should be removed are listed in m_dead_arrays.
	* This method deletes these subarrays and removes them from the m_arrays.
	* The m_dead_arrays list is cleared when done.
	*/
	SubArrayMap::iterator findit;
	for (list<SubArray*>::const_iterator it = m_dead_arrays.begin(); it != m_dead_arrays.end(); ++it) {
		/* Remove from the m_arrays map*/
		findit = m_arrays.find((*it)->getName());
		if (findit != m_arrays.end()) {
			LOG_DEBUG_STR("undertaker:Removing subarray " << (*it)->getName());
			m_arrays.erase(findit);
		} else {
			LOG_FATAL_STR("trying to remove non-existing subarray " << (*findit).second->getName());
			exit(EXIT_FAILURE);
		}

		delete (*it);
	}
	m_dead_arrays.clear();
}

//
// getByName(name)
//
SubArray* SubArrays::getByName(const string& name)
{
	// find in m_new_arrays
	SubArrayMap::const_iterator it = m_new_arrays.find(name);

	if (it != m_new_arrays.end()) {
		return (*it).second;
	}

	// if not found in m_new_arrays, try to find in m_arrays
	it = m_arrays.find(name);

	if (it != m_arrays.end()) {
		return (*it).second;
	}
	return 0;
}

//
// updateAll()
//
void SubArrays::updateAll()
{
	for (SubArrayMap::const_iterator it = m_arrays.begin();
					it != m_arrays.end(); ++it) {
		SubArray* subarray = (*it).second;

		// notify subarrays that have completed calibration
		if (subarray) {
			if (subarray->isDone()) {
				subarray->notify();
				subarray->clearDone(); // we've notified all subscribers, clear done flag
			}
		}
	}
}

//
// calibrate(CalI/F*, ACC)
//
void SubArrays::calibrate(CalibrationInterface* cal, ACC& acc, bool writeToFile, const string& dataDir)
{
	bool done = false;

	mutex_lock();
	if (acc.isValid()) {
		for (SubArrayMap::const_iterator it = m_arrays.begin();
					it != m_arrays.end(); ++it) {
			SubArray* subarray = (*it).second;
			ASSERT(0 != subarray);

			if (!subarray->isDone()) {
				LOG_INFO_STR("start calibration of subarray: " << subarray->getName());
				subarray->calibrate(cal, acc);
				LOG_INFO_STR("finished calibration of subarray: " << subarray->getName());
				done = true;
			}
			if (writeToFile) {
				writeGains(subarray, dataDir);
			}
		}
	}
	else {
		LOG_DEBUG("SubArrays::calibrate: acc is invalid");
	}
	mutex_unlock();

	//
	// prevent reuse of this acc by next calibrate call
	// TODO: this should be done elsewhere once calibrate
	// is running in its own thread
	//
	if (done) {
		acc.invalidate();
	}
}

//
// remove(name)
//
bool SubArrays::remove(const string& name)
{
	// find SubArray
	SubArrayMap::iterator it = m_arrays.find(name);

	// if found then remove
	if (it == m_arrays.end()) {
		return (false);
	}

	LOG_DEBUG_STR("remove:Removing subArray " << name);
	if ((*it).second) {
		delete ((*it).second);
	}
	m_arrays.erase(it);

	return (true);
}

//
// remove(SubArray*)
//
bool SubArrays::remove(SubArray*& subarray)
{
	return (subarray?remove(subarray->getName()):false);
}

SubArrayMap	SubArrays::getSubArrays(const string&	optionalName) {
	// NOTE
	//
	// the administration of the subarrays is unfortunately not very simple.
	// when a new beam is started the beam is add to the m_new_subbarrays.
	// it stays there until the collection of the current ACM is finished
	// (worst case 511 seconds). When the ACM buffers are swapped the new
	// subarrays are added to the actual list.
	// The removal of stopped beams goes in the same way, when a beam is stopped
	// the beam is added to the m_dead_list but stays active in the calibration
	// cycle. At the swap of the ACM buffers the subarrays whos name is in the
	// dead-list are remove from the active m_subarrays.

	SubArrayMap	answer;

	if (optionalName.empty()) {         // no name specified?
		LOG_DEBUG("getSubArray: delivering all subarrays");
		// copy whole new array
		answer = m_new_arrays;
		// add active array
		SubArrayMap::const_iterator		iter = m_arrays.begin();
		SubArrayMap::const_iterator		end  = m_arrays.end();
		while (iter != end) {
			answer[iter->first] = iter->second;
			++iter;
		}
	}
	else {
		// get subarray by name
		SubArrayMap::const_iterator		subarray;
		if (((subarray = m_arrays.find(optionalName)) != m_arrays.end()) || 
			((subarray = m_new_arrays.find(optionalName)) != m_new_arrays.end()))  {
			answer[optionalName] = subarray->second;
		}
		else {
			LOG_DEBUG_STR("getSubArray: Subarray " << optionalName << " not found");
		}
	}

	// substract deadlist from constructed answer.
	list<SubArray*>::const_iterator		iter = m_dead_arrays.begin();
	list<SubArray*>::const_iterator		end  = m_dead_arrays.end();
	while (iter != end) {
		// try to find each deadlist-name in the answer
		SubArrayMap::const_iterator	saIter = answer.find((*iter)->getName());
		if (saIter != answer.end()) {
			LOG_DEBUG_STR("SubArray " << (*iter)->getName() << " on deadlist");
			answer.erase((*iter)->getName());
		}
		++iter;
	}

	return (answer);
}

//
// writeGains(Subarray*		Subarray, dataDir)
//
void SubArrays::writeGains(SubArray*	anSubArr, const string&	dataDir)
{
	time_t now = time(0);
	struct tm* t = gmtime(&now);
	char filename[PATH_MAX];
	AntennaGains*	gains;
	anSubArr->getGains(gains, SubArray::FRONT);

	snprintf(filename, PATH_MAX, "%s/%s_%04d%02d%02d_%02d%02d%02d_gain_%dx%dx%d.dat",
						dataDir.c_str(),
						anSubArr->getName().c_str(),
						t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
						t->tm_hour, t->tm_min, t->tm_sec,
						gains->getGains().extent(firstDim),
						gains->getGains().extent(secondDim),
						gains->getGains().extent(thirdDim));
	LOG_DEBUG_STR("writeGains(" << anSubArr->getName() << ") to " << filename);

	FILE* gainFile = fopen(filename, "w");

	if (!gainFile) {
		LOG_ERROR_STR("failed to open file: " << filename);
		return;
	}

	if (fwrite(gains->getGains().data(), sizeof(complex<double>), gains->getGains().size(), gainFile) != 
			(size_t)gains->getGains().size()) {
		LOG_ERROR_STR("failed to write to file: " << filename);
	}

	(void)fclose(gainFile);
}

  } // namespace CAL
} // namespace LOFAR
