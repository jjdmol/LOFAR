//#  APLUtilities.cc: Utility functions
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
#include <Common/StringUtil.h>
#include <unistd.h>
#include <climits>
#include <cstdio>
#include <cstring>

#include "APL/APLCommon/APLUtilities.h"

namespace LOFAR {
  namespace APLCommon {

APLUtilities::APLUtilities()
{
}


APLUtilities::~APLUtilities()
{
}


time_t APLUtilities::getUTCtime()
{
  return time(0);// current system time in UTC
}

time_t APLUtilities::decodeTimeString(const string& timeStr)
{
	// empty string or -1? -> sometime in the future
	if (timeStr.empty() || (timeStr.find("-1") != string::npos)) {
		return (INT_MAX);
	}

	// does string contain a +? --> add to current time.
	string::size_type plusPos = timeStr.find('+');
	if (plusPos != string::npos) {
		return (APLUtilities::getUTCtime() + atoi(timeStr.substr(plusPos+1).c_str()));
	}

	// specified times are in UTC, seconds since 1-1-1970
	time_t	theTime = atoi(timeStr.c_str());

	// is time 0? return current time.
	if (theTime == 0) {
		return (getUTCtime());
	}

	return(atoi(timeStr.c_str()));	// return specified time.
}

//
// byteSize
//
string byteSize(double	nrBytes)
{
	double	giga (1024.0*1024.0*1024.0);
	if (nrBytes >= giga) {
		return (formatString ("%.1lfGB", nrBytes/giga));
	}

	if (nrBytes >= 1048576) {
		return (formatString ("%.1fMB", (nrBytes*1.0/1048576)));
	}

	if (nrBytes >= 1024) {
		return (formatString ("%.1fKB", (nrBytes*1.0/1024)));
	}

	return (formatString ("%.0fB", nrBytes));
}


  } // namespace APLCommon
} // namespace LOFAR

