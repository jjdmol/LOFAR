//#  NsTimestamp.h: implementation of the NsTimestamp class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
#include <Common/NsTimestamp.h>

#include <iostream>
#include <math.h>
#include <time.h>

namespace LOFAR {

void NsTimestamp::setNow(double delay)
{
	struct	timeval		tv;
	(void)gettimeofday(&tv, NULL);
	itsSec = tv.tv_sec + (long)trunc(delay);
	itsNsec = (int64)1e3 * tv.tv_usec + (long)(1e9 * (delay - trunc(delay)));
	if (itsNsec > (long)(1e9)) {
		itsNsec -= (long)(1e9);
		itsSec++;
	}
	else if (itsNsec < 0) {		// when delay is negative.
		itsNsec += (int64) 1e9;
		itsSec--;
	}
}

} // namespace LOFAR

std::ostream& LOFAR::operator<< (std::ostream& os, const NsTimestamp& ts)
{
  char timestring[64];
  char zonestring[16];
  time_t seconds = (time_t)ts.sec();

  struct tm tm;
  gmtime_r(&seconds, &tm);
  if (strftime(timestring, sizeof(timestring), "%s - %a, %d %b %Y %H:%M:%S", &tm) == 0) {
    strncpy(timestring, "unk timestamp", sizeof(timestring));
    timestring[sizeof(timestring)-1] = '\0'; // defensive
  }
  if (strftime(zonestring, sizeof(zonestring), "  %z", &tm) == 0) {
    strncpy(zonestring, "unk time zone", sizeof(zonestring));
    zonestring[sizeof(zonestring)-1] = '\0'; // defensive
  }
  return os << timestring << formatString(".%09d", ts.nsec()) << zonestring;
}

