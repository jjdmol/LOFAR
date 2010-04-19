//#  AnalogueBeam.h: implementation of the AnalogueBeam class
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
#include "AnalogueBeam.h"

#include <math.h>
#include <iostream>
#include <sys/time.h>
#include <queue>

#include <blitz/array.h>

using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace IBS_Protocol;
using namespace std;
using namespace RTC;

//
// AnalogueBeam(name, subarray, nrSubbands)
//
AnalogueBeam::AnalogueBeam(const string& 				name, 
						   const string&				antennaSet,
						   const bitset<MAX_RCUS>&		rcuMask,
						   uint							rankNr) :
	Beam		(name, antennaSet, rcuMask),
	itsRankNr	(rankNr)
{}

//
// ~AnalogueBeam
//
AnalogueBeam::~AnalogueBeam()
{
}

