//#  Observation.h: class/struct that holds the Observation information
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

#ifndef LOFAR_APPLCOMMON_OBSERVATION_H
#define LOFAR_APPLCOMMON_OBSERVATION_H

// \file
// class/struct that holds the Observation information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <APS/ParameterSet.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR {

// \addtogroup ApplCommon
// @{

//# Forward Declarations
//class forward;


// The Observation class is a public struct that can be used as base-class
// for holding Observation related information.
// It can be instantiated with a parset containing Observation information.
class Observation
{
public:
	Observation();
	~Observation();
	explicit	Observation (ACC::APS::ParameterSet*		aParSet);

	// global function for converting filtername to nyquist zone
	static uint32 nyquistzoneFromFilter(const string&	filterName);

	// get name of a beam (idx starts at 0)
	string getBeamName(uint32	beamIdx) const;

	// for operator <<
	ostream& print (ostream&	os) const;

	// data types
	typedef bitset<256> 	  RCUset_t;

	class Beam {
	public:
		Beam() {};
		~Beam() {};

		// NOTE: since not other sw in the signal chain supports switching the beam to another direction
		//		 we only support 1 direction per beam for now.
		double			angle1;
		double			angle2;
		string			directionType;
//		string			angleTimes;
		vector<int32>	subbands;
		vector<int32>	beamlets;
	};

	//# Datamembers
	string			name;
	int32			obsID;
	time_t			startTime;
	time_t			stopTime;
	uint32			nyquistZone;
	vector<string>	stations;
	int32			sampleClock;
	string			filter;
	string			MSNameMask;

	// old way of specifying antennas
	string			antennaArray;
	RCUset_t		RCUset;				// set with participating receivers

	// new way of selecting antennas
	string			antennaSet;
	bool			useLongBaselines;

	vector<Beam>	beams;
	vector<int32>	beamlet2beams;		// to which beam each beamlet belongs
	vector<int32>	beamlet2subbands;	// which subband each beamlet uses.

	// couple of values of the virtual instrument as compacted strings
	string			receiverList;
	string			stationList;
	string			BGLNodeList;
	string			storageNodeList;

};

//#
//# operator <<
//#
inline ostream& operator<< (ostream&	os, const Observation&	anObs)
{
	return (anObs.print(os));
}

// @}
} // namespace LOFAR

#endif
