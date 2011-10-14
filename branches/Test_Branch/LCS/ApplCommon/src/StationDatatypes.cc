//# StationDatatypes.cc: Several 'deployment' related functions
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
//# $Id: StationDatatypes.cc 15652 2010-05-10 14:56:33Z loose $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <ApplCommon/StationDatatypes.h>

namespace LOFAR {

// Antenna2RCUmask(antennaMask)
// Returns the corresponding RCUmask
RCUmask_t		Antenna2RCUmask(const AntennaMask_t&	am)
{
	RCUmask_t		rm;

	for (int i = 0; i < MAX_ANTENNAS; ++i) {
		if (am[i]) {
			rm.set(2*i);
			rm.set(2*i+1);
		}
	}
	return (rm);
}


// RCU2AntennaMask(RCUmask)
// Returns the corresponding AntennaMask
AntennaMask_t	RCU2AntennaMask(const RCUmask_t&		rm)
{
	AntennaMask_t	am;

	for (int i = 0; i < MAX_RCUS; i+=2) {
		if (rm[i] || rm[i+1]) {
			am.set(i/2);
		}
	}

	return (am);
}


} // namespace LOFAR
