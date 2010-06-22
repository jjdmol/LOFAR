//#  AntennaSets.h: Class to manage the antenna subsets.
//#
//#  Copyright (C) 2009
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

#ifndef LOFAR_APLCOMMON_ANTENNASET_H
#define LOFAR_APLCOMMON_ANTENNASET_H

// \file AntennaSets.h
// Class to manage the antenna subsets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarConstants.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <ApplCommon/StationInfo.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace APLCommon {

// \addtogroup package
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// class_description
// ...
class AntennaSets
{
public:
	explicit AntennaSets (const string&	filename);
	virtual ~AntennaSets();

	// Everthing about the bits
	string				RCUinputs    (const string&		setName, uint stnType = stationTypeValue()) const;
	bitset<MAX_RCUS>	LBAallocation(const string&		setName, uint stnType = stationTypeValue()) const;
	bitset<MAX_RCUS>	HBAallocation(const string&		setName, uint stnType = stationTypeValue()) const;

	// Everthing about the names
	bool				isAntennaSet (const string&	setName) const;
	vector<string>		antennaSetList() const;

	// Type of antennas used in the set
	bool			usesLBAfield(const string&	setName, uint stnType = stationTypeValue()) const;
	const string	antennaField(const string&	setName, uint stnType = stationTypeValue()) const;

	// ... example
	ostream& print (ostream& os) const
	{	return (os); }

private:
	// Copying is not allowed
	AntennaSets();
	AntennaSets(const AntennaSets&	that);
	AntennaSets& operator=(const AntennaSets& that);

	// internal datastructures
	class singleSet {
	public:
		string				antennaField;
		string				RCUinputs;
		bitset<MAX_RCUS>	LBAallocation;
		bitset<MAX_RCUS>	HBAallocation;

		singleSet() { RCUinputs.resize(MAX_RCUS,'.'); }
	};
	class setTriple {
	public:
		singleSet		europe;
		singleSet		remote;
		singleSet		core;
	};
	
	// internal functions
	bool _adoptSelector(const string&	selector, const string& antennaField, singleSet&	triple, uint rcuCount);

	//# --- Datamembers ---
	// name of the file that contains the antennaSets.
	string				itsAntennaSetFile;

	typedef	map<string, setTriple>::const_iterator	AntSetIter;

	map<string, setTriple>		itsDefinitions;
};

// Make one instance of the AntennaSets globally accessable.
AntennaSets* 	globalAntennaSets();

//# --- Inline functions ---

// ... example
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const AntennaSets& anSet)
{	
	return (anSet.print(os));
}


// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
