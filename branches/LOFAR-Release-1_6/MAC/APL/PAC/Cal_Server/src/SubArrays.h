//#  -*- mode: c++ -*-
//#  SubArrays.h: class definition for the SubArray class
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

#ifndef SUBARRAYS_H_
#define SUBARRAYS_H_

#include <APL/CAL_Protocol/SubArray.h>

namespace LOFAR {
  using EPA_Protocol::MEPHeader;
  namespace CAL {

// forward declarations
class ACC;
class CalibrationInterface;

class SubArrays : public SharedResource
{
public:
	SubArrays();
	virtual ~SubArrays();

	// @pre array != 0
	// Schedule a new array for addition to the subarray list.
	// @param array Pointer (not 0) to the array to be added.
	void schedule_add(SubArray* array);

	// Schedule subarray for removal. This is used
	// to keep a subarray while the calibration algorithm
	// is running on a separate thread. Only really remove
	// the subarray when the thread has finished.
	bool schedule_remove(const string& name);
	bool schedule_remove(SubArray*& subarray);

	// New subarrays are listed in m_new_arrays.
	// This method moves these subarrays to the m_arrays map.
	void creator();

	// Subarrays that should be removed are listed in m_dead_arrays.
	// This method deletes these subarrays removes
	// them from the m_arrays.
	void undertaker();

	// Find a subarray by name.
	// @param name Find the subarray with this name.
	// @return pointer to the subarray if found, 0 otherwise.
	SubArray* getByName(const string& name);

	// This is called periodically to check whether any of the subarrays
	// have completed calibration and need to inform their subscribers of
	// the new calibration weights.
	void updateAll();

	// Try to start calibration on all subarrays with the give
	// algorithm and array correlation cube.
	// @param cal Pointer to the calibration algorithm interface.
	// @param acc Reference to the array correlation cube to use for
	// calibration.
	void calibrate(CalibrationInterface* cal, ACC& acc, bool writeToFile, const string& dataDir);

	// Get a map with all active SubArrays.
	SubArrayMap	getSubArrays(const string&	optionalName);

	// Write the calculated gains and qualitites to a file.
	void writeGains(SubArray*	SubArr, const string&	dataDir);

private:
	// Remove subarray with the given name from the subarrays.
	// The subarray instance will be deleted.
	// @param name Name of the subarray to remove.
	// @return true if found and removed, false otherwise
	bool remove(const string& name);
	bool remove(SubArray*& subarray);

	SubArrayMap 		m_new_arrays; 	// subarrays that should be added 
										// for the next run of the 
										// calibration algorithm
	SubArrayMap			m_arrays;
	list<SubArray*>    	m_dead_arrays; 	// subarrays that have been stopped
};

  }; // namespace CAL
}; // namespace LOFAR

#endif 

