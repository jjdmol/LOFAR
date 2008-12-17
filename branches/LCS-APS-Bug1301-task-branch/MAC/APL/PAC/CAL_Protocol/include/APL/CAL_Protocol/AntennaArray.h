//#  -*- mode: c++ -*-
//#  AntennaArray.h: class definition for the AntennaArray class
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

#ifndef ANTENNAARRAY_H_
#define ANTENNAARRAY_H_

#include <blitz/array.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

#include <Common/LofarTypes.h>

namespace LOFAR {
  namespace CAL {

/**
* This class represents an antenna array. The LOFAR remote station will initially
* have two anntena arrays. One for the low-band antennas, and one for the high-band
* antennas. Alternative configurations of the antennas would also be described by 
* separate AntennaArray instance.
* The AntennaArray instantes are created using information from a configuration file.
*/
class AntennaArray
{
public:
	// Create a new antenna array.
	// @param name      The name of this antenna array.
	// @param pos       The x,y,z positions of the antennas (dimensions: nantennas x npol (2) x 3 (x,y,z)
	// @param rcuindex  The mapping from antenna to input RCU. If the mapping is one-on-one then this
	// parameter can be omitted. This is used for creating subarrays of an AntennaArray.
	AntennaArray(); // only used by CAL_Protocol.prot, should not be used directly
	AntennaArray(string                    name,
	const blitz::Array<double, 1>& geoloc,
	const blitz::Array<double, 3>& pos,
	const blitz::Array<int16,  2>* rcuindex = 0);
	virtual ~AntennaArray();

	// Get the name of the array.
	// @return The name of the array or subarray (e.g. LBA_ARRAY, HBA_ARRAY, SINGLEPOL_LBA_ARRAY, etc).
	string getName() const { return m_name; }

	// Get geographical location.
	// @return geographical location as 1-dimensional blitz array with three elements.
	const blitz::Array<double, 1>& getGeoLoc() const { return m_geoloc; }

	// Get the positions of the antennas.
	// @return The array with positions of the antennas.
	const blitz::Array<double, 3>& getAntennaPos() const { return m_pos; }

	// @return The number of dual polarized antennas (one antenna = two dipoles)
	int getNumAntennas() const { return m_pos.extent(blitz::firstDim); }

	// Get the RCU index of a particular antenna.
	inline int16 getRCUIndex(int nantenna, int npol) const { return m_rcuindex(nantenna, npol); }

	// Assignment operator
	AntennaArray& operator=(const AntennaArray& other);

protected:
	/* prevent copy */
	AntennaArray(const AntennaArray& other); // no implementation

	string             		m_name;     // name of this antenna array
	blitz::Array<double, 1> m_geoloc;   // geographical location
	blitz::Array<double, 3> m_pos;      // three dimensions, Nantennas x Npol x 3 (x,y,z)
	blitz::Array<int16, 2>  m_rcuindex; // the index of the rcu to which a dipole is connected, dimensions Nantennas x Npol
	};

//
// Factory class for the Antenna class. Manages one or more AntennaArrays which are loaded from file.
//
class AntennaArrays
{
public:
	AntennaArrays();
	virtual ~AntennaArrays();

	// Get all antenna arrays from the given filename
	// @param url Load antenna arrays from this resource location (e.g. filename or database table).
	void getAll(string url);

	// Get an antenna array by name.
	// @return a pointer to the AntennaArray or 0 if the array was not found.
	const AntennaArray* getByName(string name);

	// Get a list of ames of all loaded antennaarrays
	// @return a pointer to the AntennaArray or 0 if the array was not found.
	vector<string> getNameList();

private:
	map<string, const AntennaArray*> m_arrays;
};

  }; // namespace CAL
}; // namespace LOFAR

#endif /* ANTENNAARRAY_H_ */

