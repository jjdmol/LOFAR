//#  StationInfo.h:	Several 'deployment' related routines.
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

#ifndef LOFAR_DEPLOYMENT_STATIONINFO_H
#define LOFAR_DEPLOYMENT_STATIONINFO_H

// \file StationInfo
// Several 'deployment' related routines

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

//# Avoid 'using namespace' in headerfiles

namespace LOFAR {

// \addtogroup ApplCommon
// @{

#define LOFAR_BASE_LOCATION		"/opt/lofar"
#define LOFAR_BIN_LOCATION		"/opt/lofar/bin"
#define LOFAR_CONFIG_LOCATION	"/opt/lofar/etc"
#define LOFAR_SHARE_LOCATION	"/opt/lofar/share"
#define LOFAR_LOG_LOCATION		"/opt/lofar/log"

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
string	stationTypeStr();									// CS, RS, ES
string	stationRingName();									// Core, Remote, Europe
string	PVSSDatabaseName(const string&	someName = "");		// hostname w/o CUtype
string	realHostname(const string&	someName);				// adds 'C' when it is missing.

string  PVSS2SASname(const string& PVSSname);				// convert PVSS DPname to SAS DPname
string  SAS2PVSSname(const string& SASname);				// convert SAS DPname to PVSS DPname


// @}
} // namespace LOFAR

#endif
