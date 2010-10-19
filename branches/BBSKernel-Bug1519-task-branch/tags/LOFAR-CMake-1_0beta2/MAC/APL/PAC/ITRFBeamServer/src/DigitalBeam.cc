//#  DigitalBeam.h: implementation of the DigitalBeam class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>

#include <APL/RTCCommon/PSAccess.h>
#include "BeamServerConstants.h"
#include "DigitalBeam.h"

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>

#include <fcntl.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace IBS_Protocol;
using namespace std;
using namespace RTC;

//
// DigitalBeam(name, subarray, nrSubbands)
//
DigitalBeam::DigitalBeam(const string& 					name, 
					     const string& 					antennaSet, 
					     const Beamlet2SubbandMap&		allocation, 
					     const bitset<MAX_RCUS>&		rcuMask,
					     uint							ringNr) :
	Beam 				(name, rcuMask),
	itsAntennaSet		(antennaSet),
	itsBeamletAllocation(allocation),
	itsRingNr			(ringNr)
{}

//
// ~DigitalBeam
//
DigitalBeam::~DigitalBeam()
{
	deallocate();
}

//
// deallocate()
//
void DigitalBeam::deallocate()
{
	// clear allocation
	itsBeamletAllocation().clear();
}

#if 0
//
// setSubarray(array)
//
void DigitalBeam::setSubarray(const CAL::SubArray& array)
{
	itsSubArray = array;
}
#endif

