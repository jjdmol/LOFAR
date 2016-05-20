//#  -*- mode: c++ -*-
//#  SubArrayMgr.h: class definition for the SubArray class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: SubArrayMgr.h 11768 2008-09-17 14:18:33Z overeem $

#ifndef SUBARRAYMGR_H_
#define SUBARRAYMGR_H_

#include <Common/lofar_list.h>
#include <Common/LofarConstants.h>
#include <GCF/TM/GCF_PortInterface.h>
#include <APL/CAL_Protocol/SubArray.h>
#include "SharedResource.h"
#include "ACC.h"

namespace LOFAR {
  using GCF::TM::GCFPortInterface;
  namespace CAL {

// forward declarations
class ACC;

class SubArrayMgr : public SharedResource
{
public:
	SubArrayMgr();
	virtual ~SubArrayMgr();

	// @pre array != 0
	// Schedule a new array for addition to the subarray list.
	// @param array Pointer (not 0) to the array to be added.
	void scheduleAdd(SubArray* array, GCFPortInterface* port);

	// Schedule subarray for removal. This is used
	// to keep a subarray while the calibration algorithm
	// is running on a separate thread. Only really remove
	// the subarray when the thread has finished.
	bool scheduleRemove(const string& name);
	bool scheduleRemove(SubArray*& subarray);

	// New subarrays are listed in itsNewArrays.
	// This method moves these subarrays to the itsActiveArrays map.
	void activateArrays();

	// Subarrays that should be removed are listed in itsDeadArrays.
	// This method deletes these subarrays removes
	// them from the itsActiveArrays.
	void removeDeadArrays();

	// Get a RCU mask of all RCU's that are used for the given rcumode.
	// @param rcumode number 1..7 being one of the LOFAR receiver modes
	// @return An bitmask containing all RCU's that are active for the rcumode.
	RCUmask_t getRCUs(uint	rcumode) const;

	// Find a subarray by name.
	// @param name Find the subarray with this name.
	// @return pointer to the subarray if found, 0 otherwise.
	SubArray* getByName(const string& name);

	// Get a map with all active SubArrays.
	SubArrayMap	getSubArrays(const string&	optionalName);

	// Subscription administration
	void addSubscription(const string& subArrayName, GCFPortInterface*	port);
	bool removeSubscription(const string& subArrayName);

	// Send the given gains to all subbands that use the given rcumode.
	// @param rcumode number 1..7 being one of the LOFAR receiver modes
	// @param gains an AntennaGain class containing the calibrated gains
	void publishGains(uint rcumode, const AntennaGains& theGains);

    // Try to start calibration on all subarrays with the give
	// algorithm and array correlation cube.
	// @param cal Pointer to the calibration algorithm interface.
	// @param acc Reference to the array correlation cube to use for
	// calibration.
	void calibrate(ACC& acc, bool writeToFile);

private:
	// Note: SubArrayMap : map<string, SubArray*>
	SubArrayMap 					itsNewArrays; 	// subarrays that should be added
													// for the next run of the
													// calibration algorithm
	list<SubArray*>    				itsDeadArrays; 	// subarrays that have been stopped
	SubArrayMap						itsActiveArrays;// Arrays currently being calibrated.
	map<string, GCFPortInterface*>	itsPortMap;
	map<string, GCFPortInterface*>	itsSubscriptionMap;
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif

