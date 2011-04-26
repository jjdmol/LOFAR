//# StationDataTypes.h.h:	Several 'deployment' related routines.
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
//# $Id: StationDataTypes.h.h 14057 2009-09-18 12:26:29Z diepen $

#ifndef LOFAR_APPLCOMMON_STATIONDATATYPES_H
#define LOFAR_APPLCOMMON_STATIONDATATYPES_H

// \file StationDataTypes.h
// Several station related datatypes and conversions

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarConstants.h>

//# Avoid 'using namespace' in headerfiles

namespace LOFAR {

// \addtogroup ApplCommon
// @{

typedef bitset<MAX_RCUS>		RCUmask_t;
typedef bitset<MAX_ANTENNAS>	AntennaMask_t;
typedef bitset<MAX_SUBBANDS>	SubbandMask_t;
typedef bitset<MAX_BEAMLETS>	BeamletMask_t;

using LOFAR::RCUmask_t;
using LOFAR::AntennaMask_t;
using LOFAR::SubbandMask_t;
using LOFAR::BeamletMask_t;

RCUmask_t		Antenna2RCUmask(const AntennaMask_t&	am);
AntennaMask_t	RCU2AntennaMask(const RCUmask_t&		rm);

// @}
} // namespace LOFAR

#endif
