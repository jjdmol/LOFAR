//#  -*- mode: c++ -*-
//#  SubArray.h: class definition for the SubArray class
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
//#  $Id: SubArray.h 10637 2007-11-05 10:37:26Z overeem $

#ifndef ICALPROTOCOL_SUBARRAY_H_
#define ICALPROTOCOL_SUBARRAY_H_

#include <blitz/array.h>

#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_bitset.h>
#include <ApplCommon/StationDatatypes.h>

#include <APL/ICAL_Protocol/SpectralWindow.h>
#include <APL/ICAL_Protocol/AntennaGains.h>

namespace LOFAR {
  namespace ICAL {

// forward declarations

class SubArray
{
public:
	// default constructor
	SubArray();	// needed for packing/unpacking

	// Construct a subarray.
	// @param name   The name of the subarray.
	// @param geoloc The geographical location of the subarray.
	// @param pos    The antenna positions of the parent array elements (nantennas x npolarizations x 3-coordinates).
	// @param select Select for each polarization dipole of each antenna whether it is included (true) in the subarray.
	// @param sampling_frequency The sampling frequency this runs at.
	// @param nyquist_zone The nyquist zone in which we wish to measure.
	// @param nsubbands The number of subbands of the spectral window.
	// @param rcucontrol The RCU control setting (LB, HBL, HBH, etc).
	SubArray(const string&             		name,
			 const string&					antennaSet,
			 RCUmask_t						RCUmask,
			 bool							LBAfilterOn,
			 double                         sampling_frequency,
			 int                            nyquist_zone);
	SubArray(const string& name); // used to return unknown subarray
	~SubArray();

	// Start (background) calibration of the subarray
	// using the specified algorithm and ACC as input.
	// @param cal The calibration algorithm to use.
	// @param acc The Array Correlation Cube on which to calibrate.
	void updateGains(const AntennaGains&	newGains);

	// Get calibration result (if available).
	// @param cal Calibration result
	AntennaGains& getGains() 
		{ return (*itsGains); }

	// Write gains to a file
	void writeGains();

	// get name of Subarray
	const string& name() const	
		{ return (itsName); }

	// get antennaSetname of Subarray
	const string& antennaSet() const	
		{ return (itsAntennaSet); }

	// get bitset containing the rcu's of the subArray.
	RCUmask_t	 RCUMask(uint	rcumode) const;
	RCUmask_t	 RCUMask() const	
		{ return (itsRCUmask); }

	// Get a reference to the spectral window for this subarray.
	const SpectralWindow& SPW() const 
		{ return (itsSPW); }

	// Get a reference to the RCUmodes
	const blitz::Array<uint,1>& RCUmodes() const 
		{ return (itsRCUmodes); }

	// Test if RCUmode is used by this array
	bool usesRCUmode(int	rcumode) const;

	// Assignment operator.
	SubArray& operator=(const SubArray& rhs);

	//@{
	// marshalling methods
	unsigned int getSize();
	unsigned int pack   (void* buffer);
	unsigned int unpack (void* buffer);
	//@}

	// call for operator<<
	ostream& print (ostream& os) const;

private:
	// prevent copy 
	SubArray(const SubArray& other); // no implementation

	string					itsName;			// unique name choosen by user.
	string					itsAntennaSet;		// name of the used antennaSet
	SpectralWindow 			itsSPW;				// the spectral window for this subarray
	RCUmask_t				itsRCUmask;			// used RCU's

	blitz::Array<uint,1>	itsRCUmodes;		// redundant info (=AntSet+SPW for each rcu in RCUmask)
	bitset<NR_RCU_MODES+1>	itsRCUuseFlags;		// which modes are used.
//	bool					itsLBAfilterOn;
	AntennaGains*			itsGains;			// calibration result record
};

// operator<<
inline ostream& operator<< (ostream& os, const SubArray& anSA)
{
	return (anSA.print(os));
}


// ------------------- SubArraymap -------------------
//
// Makes map<string, SubArray*> stremable.
class SubArrayMap : public map<string, SubArray*>
{
public:
	//@{
	// marshalling methods
	unsigned int getSize();
	unsigned int pack   (void* buffer);
	unsigned int unpack (void* buffer);
	//@}
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif 

