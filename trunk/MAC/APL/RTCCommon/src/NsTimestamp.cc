//#  NsTimestamp.h: implementation of the NsTimestamp class
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
//#  $Id: NsTimestamp.cc 6858 2005-10-21 12:27:59Z wierenga $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <APL/RTCCommon/NsTimestamp.h>

#include <math.h>
#include <time.h>

//using namespace LOFAR;
//using namespace RTC;
//using namespace std;

namespace LOFAR {
  namespace RTC {

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

  } // namepsace RTC
} // namespace LOFAR

std::ostream& LOFAR::RTC::operator<< (std::ostream& os, const NsTimestamp& ts)
{
  char timestring[256];
  char zonestring[16];
  time_t seconds = (time_t)ts.sec();

  strftime(timestring, 255, "%s - %a, %d %b %Y %H:%M:%S", gmtime(&seconds));
  strftime(zonestring, 15, "  %z", gmtime(&seconds));
  return os << timestring << formatString(".%09d", ts.nsec()) << zonestring;
}

