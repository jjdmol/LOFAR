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

#ifndef LOFAR_APLCOMMON_OBSERVATION_H
#define LOFAR_APLCOMMON_OBSERVATION_H

// \file
// class/struct that holds the Observation information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <APS/ParameterSet.h>
#include <Common/lofar_bitset.h>

namespace LOFAR {
  namespace APLCommon {

// \addtogroup APLCommon
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

	typedef bitset<256> 	  RCUset_t;

	//# Datamembers
	string			name;
	int32			treeID;
	time_t			startTime;
	time_t			stopTime;
	int16			nyquistZone;
	vector<int16>	subbands;
	vector<int16>	beamlets;
	int32			sampleClock;
	string			filter;
	string			antennaArray;
	RCUset_t		RCUset;			// set with participating receivers
};

// @}

  } // namespace APLCommon
} // namespace LOFAR

#endif
