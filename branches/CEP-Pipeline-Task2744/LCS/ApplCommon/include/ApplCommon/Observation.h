//# Observation.h: class/struct that holds the Observation information
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

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
	explicit	Observation (ParameterSet*		aParSet, bool	hasDualHBA = false);

	// global function for converting filtername to nyquist zone
	static uint nyquistzoneFromFilter(const string&	filterName);

	// get name of a beam (idx starts at 0)
	string getBeamName(uint	beamIdx) const;
	string getAnaBeamName() const;

	// check if the given Observation conflicts with this one
	bool	conflicts(const	Observation&	other) const;

	// Returns a bitset containing the RCU's requested by the observation.
	bitset<MAX_RCUS> getRCUbitset(int nrLBAs, int nrHBAs, const string& anAntennaSet);
	// TEMP HACK
	string getAntennaArrayName(bool hasSplitters) const;

	// Support for dynamic dataslot allocation
	vector<int>	getBeamAllocation(const string&	stationName = "") const;
	vector<int>	getBeamlets (uint beamIdx, const string&	stationName = "") const;

	// for operator <<
	ostream& print (ostream&	os) const;

	// data types
	typedef bitset<MAX_RCUS> 	  RCUset_t;

	class Pointing {
	public:
		Pointing() {};
		~Pointing() {};
		double			angle1;
		double			angle2;
		string			directionType;
		time_t			startTime;
		int				duration;
	};
		
	class Beam {
	public:
		Beam() {};
		~Beam() {};
		Beam& operator=(const Beam& that) {
			if (this != &that) {
				this->name 		 = that.name;
				this->antennaSet = that.antennaSet;
				this->pointings  = that.pointings;
				this->momID 	 = that.momID;
				this->subbands 	 = that.subbands;
			}
			return (*this);
		}

		string				name;
		string				antennaSet;
		vector<Pointing>	pointings;

		int					momID;
		vector<int>			subbands;
	};

	class AnaBeam {
	public:
		AnaBeam() {};
		~AnaBeam() {};
		AnaBeam& operator=(const AnaBeam& that) {
			if (this != &that) {
				this->name 		 = that.name;
				this->antennaSet = that.antennaSet;
				this->pointings  = that.pointings;
				this->rank 		 = that.rank;
			}
			return (*this);
		};

		string				name;
		string				antennaSet;
		vector<Pointing>	pointings;

		int					rank;
	};

    class StreamToStorage {
    public:
        string dataProduct;
        unsigned streamNr;
        string filename;

        unsigned sourcePset;
        string destStorageNode;
        string destDirectory;
          
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

	// new way of selecting antennas
	string			antennaSet;			// like LBA_INNER, LBA_OUTER, etc.
	bool			useLongBaselines;
	bool			splitterOn;			// On or Off
	bool			dualMode;			// HBA_DUAL selected

	// beams
	vector<Beam>	beams;
	vector<AnaBeam>	anaBeams;

    vector<StreamToStorage> streamsToStorage; 

	// couple of values of the virtual instrument as compacted strings
	string			receiverList;
	string			stationList;
	string			BGLNodeList; 	 
	string			storageNodeList;

private:
	RCUset_t		RCUset;				// set with participating receivers, use getRCUbitset to get this value.

	// many(!) vectors for dataslot allocation
	bool			itsHasDataslots;	// old or new type of parset
	bool			itsStnHasDualHBA;	// 
	ParameterSet	itsDataslotParset;	// subset of parset with dataslotinfo for getxxxAllocation()
	vector<int>		itsSlotTemplate;	// clean template with the slots that may be used.
	vector<uint>	itsBeamSlotList;	// beamnumber vector
	vector<int>		beamlet2beams;		// OLD:to which beam each beamlet belongs
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
