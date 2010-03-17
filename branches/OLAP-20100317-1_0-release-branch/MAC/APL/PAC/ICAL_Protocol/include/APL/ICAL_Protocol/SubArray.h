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

#ifndef SUBARRAY_H_
#define SUBARRAY_H_

#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <Common/lofar_string.h>
#include <Common/lofar_bitset.h>

#include <APL/RTCCommon/Subject.h>
#include <APL/RSP_Protocol/MEPHeader.h>
#include <APL/ICAL_Protocol/SpectralWindow.h>
#include <APL/ICAL_Protocol/AntennaArray.h>
/*#include "ACC.h"*/
#include "SharedResource.h"
#include <APL/ICAL_Protocol/AntennaGains.h>

namespace LOFAR {
  using EPA_Protocol::MEPHeader;
  namespace CAL {

// forward declarations
class ACC;
class CalibrationInterface;

class SubArray : public AntennaArray, public RTC::Subject
{
public:
	// Default constructor.
	SubArray();

	// Construct a subarray.
	// @param name   The name of the subarray.
	// @param geoloc The geographical location of the subarray.
	// @param pos    The antenna positions of the parent array elements (nantennas x npolarizations x 3-coordinates).
	// @param select Select for each polarization dipole of each antenna whether it is included (true) in the subarray.
	// @param sampling_frequency The sampling frequency this runs at.
	// @param nyquist_zone The nyquist zone in which we wish to measure.
	// @param nsubbands The number of subbands of the spectral window.
	// @param rcucontrol The RCU control setting (LB, HBL, HBH, etc).
	SubArray(string                    name,
	const blitz::Array<double, 1>& geoloc,
	const blitz::Array<double, 3>& pos,
	const blitz::Array<bool, 2>&   select,
	double                         sampling_frequency,
	int                            nyquist_zone,
	int                            nsubbands,
	uint32                         rcucontrol);
	SubArray(string name); // used to return unknown subarray
	virtual ~SubArray();

	// Start (background) calibration of the subarray
	// using the specified algorithm and ACC as input.
	// @param cal The calibration algorithm to use.
	// @param acc The Array Correlation Cube on which to calibrate.
	void calibrate(CalibrationInterface* cal, ACC& acc);

	// Get calibration result (if available).
	// @param cal Calibration result
	bool getGains(AntennaGains*& cal, int buffer = FRONT);

	// get bitset containing the rcu's of the subArray.
	typedef bitset<MEPHeader::MAX_N_RCUS> 	RCUmask_t;
	RCUmask_t	 getRCUMask() const;

	// Abort background calibration.
	void abortCalibration();

	// Check whether calibration has completed.
	bool isDone();

	// Used to clear the 'done' flag after updating all subscriptions.
	void clearDone();

	// Get a reference to the spectral window for this subarray.
	const SpectralWindow& getSPW() const;

	// Assignement operator.
	SubArray& operator=(const SubArray& rhs);

	// Enumeration of buffer positions.
	enum {
		FRONT = 0,
		BACK = 1
	};

	//@{
	// marshalling methods
	unsigned int getSize();
	unsigned int pack   (void* buffer);
	unsigned int unpack (void* buffer);
	//@}

private:
	// prevent copy 
	SubArray(const SubArray& other); // no implementation

	int						m_antenna_count;		// number of seleted antennas
	blitz::Array<bool, 2>	m_antenna_selection;	// antenna selection dimensions: 
													// (nantennas x npol)
	RCUmask_t				itsRCUmask;

	SpectralWindow 			m_spw;              // the spectral window for this subarray
	AntennaGains*			m_result[BACK + 1]; // two calibration result records
};

//
// getRCUMask()
//
inline	SubArray::RCUmask_t	SubArray::getRCUMask() const
{
	return (itsRCUmask);
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

  }; // namespace CAL
}; // namespace LOFAR

#endif 

