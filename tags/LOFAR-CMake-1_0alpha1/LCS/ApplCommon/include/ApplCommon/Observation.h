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
#include <Common/ParameterSet.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/LofarConstants.h>

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
	explicit	Observation (ParameterSet*		aParSet);

	// global function for converting filtername to nyquist zone
	static uint nyquistzoneFromFilter(const string&	filterName);

	// get name of a beam (idx starts at 0)
	string getBeamName(uint	beamIdx) const;

	// check if the given Observation conflicts with this one
	bool	conflicts(const	Observation&	other) const;

	// Returns a bitset containing the RCU's requested by the observation.
	bitset<MAX_RCUS> getRCUbitset(int nrLBAs, int nrHBAs, int nrRSPs, bool hasSplitters);

	// REO: The next functions don't make sense, all info is public!!!!!
	// OLAP: get the subbandList
	vector<uint32> getSubbandList() const;
	// OLAP: get the beamList
	vector<uint32> getBeamList() const;
	// OLAP: get the rspBoardList
	vector<uint32> getRspBoardList() const;
	// OLAP: get the rspSlotList
	vector<uint32> getRspSlotList() const;
 
	// for operator <<
	ostream& print (ostream&	os) const;

	// data types
	typedef bitset<MAX_RCUS> 	  RCUset_t;

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
		vector<int>		subbands;
		vector<int>		beamlets;
	};

	//# Datamembers
	string			name;
	int				obsID;
	time_t			startTime;
	time_t			stopTime;
	uint			nyquistZone;
	vector<string>	stations;
	int				nrSlotsInFrame;
	int				sampleClock;		// 160 | 200
	string			filter;				// LBA_30_80, LBA_10_90, HBA_110_190, etc.
	string			MSNameMask;
	string			realPVSSdatapoint;

	// old way of specifying antennas
	string			antennaArray;
private:
	RCUset_t		RCUset;				// set with participating receivers, use getRCUbitset to get this value.

public:
	// new way of selecting antennas
	string			antennaSet;			// like LBA_INNER, LBA_OUTER, etc.
	bool			useLongBaselines;
	bool			splitter;			// On or Off

	vector<Beam>	beams;
	vector<int>		beamlet2beams;		// to which beam each beamlet belongs
	vector<int>		beamlet2subbands;	// which subband each beamlet uses.
	uint			nrRSPboards;		// the number of RSPboads.
	vector<uint>	subbandList;		// OLAP: subband list.
	vector<uint>	beamList;	        // OLAP: beam list.
	vector<uint>	rspBoardList;		// OLAP: RSP board list.
	vector<uint>	rspSlotList;		// OLAP: RSP Slot list
	bool			uStation;			// use uStation mode?

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
