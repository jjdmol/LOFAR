//#  Timestamp.h: implementation of the Timestamp class
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

#include <APL/RTCCommon/Timestamp.h>

#include <math.h>
#include <time.h>

using namespace LOFAR;
using namespace RTC;
using namespace std;

void Timestamp::setNow(double delay)
{
  (void)gettimeofday(&m_tv, 0);
  m_tv.tv_sec += (long)trunc(delay);

  /**
   * For future use it may be required to have higher
   * precision than seconds.
   */
#if 0
  m_tv.tv_usec = 0;
#else
  m_tv.tv_usec += (int)(10e6 * (delay - trunc(delay)));
  if (m_tv.tv_usec > (int)(10e6))
  {
      m_tv.tv_usec -= (int)(10e6);
      m_tv.tv_sec++;
  }
#endif
}

std::ostream& LOFAR::RTC::operator<< (std::ostream& os, const Timestamp& ts)
{
  char timestring[256];
  time_t seconds = (time_t)ts.sec();

  strftime(timestring, 255, "%s - %a, %d %b %Y %H:%M:%S  %z", gmtime(&seconds));
  return os << timestring; // << "." << ts.usec();
}

void Timestamp::convertToMJD(double& mjd, double& fraction)
{
  double sec = this->sec();
  sec /= double(24*3600);
  mjd = floor(sec);
  fraction = sec - mjd + this->usec() / double(1000000) / double(24*3600);
  if (fraction > 1) {
    mjd++;
    fraction--;
  }
  // 40587 modified Julian day number = 00:00:00 January 1, 1970, GMT.
  mjd += 40587;
}
