//# TimeCoord.cc: Class to hold a time coordinate as 2 values
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/TimeCoord.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <cmath>
#include <ctime>
#include <sys/time.h>

namespace LOFAR
{
  namespace AMC
  {
    
    const static double usecPerSec = double(1000000);
    const static double secPerDay  = double(24*3600);

    TimeCoord::TimeCoord()
    {
      struct timeval tp;
      gettimeofday (&tp, 0);
      double sec = tp.tv_sec;
      sec /= secPerDay;
      itsDay = floor(sec);
      itsFrac = sec - itsDay + tp.tv_usec / usecPerSec / secPerDay;
      if (itsFrac > 1) {
        itsDay++;
        itsFrac--;
      }
      // 40587 modified Julian day number = 00:00:00 January 1, 1970, GMT.
      itsDay += 40587;
    }

    // Copied from MVTime in aips++
    TimeCoord::TimeCoord (int yy, int mm, double dd,
                          double h, double m, double s)
    {
      if (mm < 3) {
        yy--;
        mm += 12;
      }
      dd += (h + (m + s/60) / 60) / 24;
      int idd = int(dd);
      itsFrac = dd - idd;
      int leapdays = 0;
      // Take care of leap days in Julian calendar.
      if (yy>1582 || (yy==1582 && (mm>10 || (mm==10 && dd >= 15)))) { 
        leapdays = int(yy/100.);
        leapdays = 2 - leapdays + int(leapdays/4);
      }
      itsDay = int(365.25*yy) + int(30.6001*(mm+1)) + idd - 679006.0 + leapdays;
    }

    TimeCoord::TimeCoord (double mjd, double fraction)
    {
      itsDay = floor(mjd);
      itsFrac = mjd - itsDay;
      double rest = floor(fraction);
      itsDay += rest;
      itsFrac += fraction - rest;
    }

    void TimeCoord::utc(double s)
    {
      double days = s / secPerDay;
      itsDay = floor(days);
      itsFrac = days - itsDay;
      itsDay += 40587;
    }

    void TimeCoord::local(double s)
    {
      s -= getUTCDiff();
      utc(s);
    }

    void TimeCoord::mjd(double mjd)
    {
      itsDay = floor(mjd);
      itsFrac = mjd - itsDay;
    }
    
    void TimeCoord::ymd (int& yyyy, int& mm, int& dd, bool local) const
    {
      double val = itsDay+itsFrac;
      if (local) {
        val += getUTCDiff() / secPerDay;
      }
      int z = int(val + 2400001.0);
      dd = z;
      if (z >= 2299161) {
        int al = int(((double) z - 1867216.25)/36524.25);
        dd = z + 1 + al - al/4;
      }
      dd += 1524;
      // tmp introduced to circumvent optimization problem with gcc2.7.2.1
      // on the DecAlpha
      int tmp = int((dd - 122.1)/365.25);
      yyyy = tmp;
      int d = int(365.25 * tmp);
      mm = tmp = int((dd - d)/30.6001);
      dd -= d + int(30.6001 * tmp); // day
      if (mm < 14) {			// month
        mm--;
      } else {
        mm -= 13;
      }
      yyyy -= 4715;			// year
      if (mm > 2) yyyy--;
    }

    void TimeCoord::hms (int& h, int& m, double& s, bool local) const
    {
      double val = itsDay+itsFrac;
      if (local) {
        val += getUTCDiff() / secPerDay;
      }
      val -= int(val);    // get fraction of day
      val *= 24;
      h = int(val);
      val -= h;
      val *= 60;
      m = int(val);
      s = 60*(val - m);
    }

    ostream& operator<< (ostream& os, const TimeCoord& time)
    {
      int yy,mm,dd,h,m;
      double s;
      time.ymd (yy, mm, dd);
      time.hms (h, m, s);
      int is = int(s);
      int us = int((s-is)*usecPerSec);
      char c(os.fill('0'));
      os << yy << '-' << setw(2) << mm << '-' << setw(2) << dd << ' ' 
         << setw(2) << h << ':' << setw(2) << m << ':' 
         << setw(2) << is << "." << setw(6) << us;
      os.fill(c);
      return os;
    }

    double TimeCoord::getUTCDiff()
    {
#ifdef HAVE_ALTZONE
      return (altzone + timezone);
#else
      int dst = 0;
      time_t tim = time (NULL);
      struct tm *tm_info = localtime (&tim);
      if (tm_info->tm_isdst > 0) {
        dst = 3600;
      }
      return dst - timezone;
#endif
    }

  } // namespace AMC

} // namespace LOFAR
