//# StationInfo.h:	Several 'deployment' related routines.
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

#ifndef LOFAR_DEPLOYMENT_STATIONINFO_H
#define LOFAR_DEPLOYMENT_STATIONINFO_H

// \file StationInfo
// Several 'deployment' related routines

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ApplCommon/LofarDirs.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

//# Avoid 'using namespace' in headerfiles

namespace LOFAR {

// \addtogroup ApplCommon
// @{

//
// Nameconventions dictate that the hostname has the following syntax:
//
// Syntax of hostname: <stationType><arm><ring><CUtype>
//	with: stationType = CS | RS | ES<countrycode>
//		  arm = 1..5   [ 1 digit  ]
//		  ring = 1..9  [ 2 digits ]
//		  CUType = C | W
//
int		stationTypeValue();									// 0..2 : for resp. error, CS, RS, ES
int		stationTypeValue (const string& name);				// 0..2 : for resp. error, CS, RS, ES
string	stationTypeStr();									// CS, RS, ES
string	stationRingName();									// Core, Remote, Europe
string	PVSSDatabaseName(const string&	someName = "");		// hostname w/o CUtype
string	realHostname(const string&	someName);				// adds 'C' when it is missing.

// Get the ObservationNr from the controllername.
uint32	getObservationNr (const string&	ObservationName);

// Get the instanceNr from the controllername.
uint16	getInstanceNr (const string&	ObservationName);

// Construct PS name solving markers line @observation@, @ring@, etc.
string	createPropertySetName(const string&		propSetMask,
							  const string&		controllerName,
							  const string&		realDPname = "REALDPNAME");

string  PVSS2SASname(const string& PVSSname);				// convert PVSS DPname to SAS DPname
string  SAS2PVSSname(const string& SASname);				// convert SAS DPname to PVSS DPname


// @}
} // namespace LOFAR

#endif
