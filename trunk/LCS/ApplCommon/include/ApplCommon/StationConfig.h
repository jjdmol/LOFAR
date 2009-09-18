//# StationConfig.h: Interface class for accessing RemoteStation.conf
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_APPLCOMMON_STATION_CONFIG_H
#define LOFAR_APPLCOMMON_STATION_CONFIG_H

// \file
// Interface class for accessing RemoteStation.conf

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_fstream.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

namespace LOFAR {

// \addtogroup ApplCommon
// @{

//# Forward Declarations
//class forward;

// The StationConfig class is an interface for the RemoteStation.conf file.
// It reads in the file and stores the values in its datamembers. All datamembers
// are public for easy access.

class StationConfig
{
public:
	StationConfig();
	~StationConfig();

private:
	// Copying is not allowed.
	StationConfig (const StationConfig& that);
	StationConfig& operator= (const StationConfig& that);

public:
	//# Data members
	int		stationID;
	int		nrRSPs;
	int		nrTBBs;
	int		nrLBAs;
	int		nrHBAs;
	bool	hasSplitters;
	bool	hasWideLBAs;
};

// @}

} // namespace LOFAR

#endif
