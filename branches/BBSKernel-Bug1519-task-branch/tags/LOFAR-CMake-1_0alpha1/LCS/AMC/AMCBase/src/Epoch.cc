//# Epoch.cc: Class to hold a time coordinate as 2 values
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
#include <AMCBase/Epoch.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>
#include <cmath>
#include <ctime>
#include <sys/time.h>

namespace LOFAR
{
  namespace AMC
  {
    
    static const double usecPerSec = double(1000000);
    static const double secPerDay  = double(24*3600);

    //################  Public functions  ################//

    Epoch::Epoch()
    {
      struct timeval tp;
      gettimeofday (&tp, 0);
      set(tp.tv_sec / secPerDay);
      // 40587 modified Julian day number = 00:00:00 January 1, 1970, GMT.
      itsDay += 40587;
    }

    // Copied from MVTime in aips++
    Epoch::Epoch (int yy, int mm, double dd, double h, double m, double s)
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
      adjust();
    }

    Epoch::Epoch (double mjd, double fraction)
    {
      set(mjd);
      add(fraction);
      adjust();
    }

    void Epoch::utc(double s)
    {
      set(s / secPerDay);
      itsDay += 40587;
    }

    void Epoch::local(double s)
    {
      s -= getUTCDiff();
      utc(s);
    }

    void Epoch::ymd (int& yyyy, int& mm, int& dd, bool local) const
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

    void Epoch::hms (int& h, int& m, double& s, bool local) const
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

    double Epoch::getUTCDiff()
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

    Epoch& Epoch::operator+=(const Epoch& that)
    {
      itsDay += that.itsDay;
      itsFrac += that.itsFrac;
      adjust();
      return *this;
    }

    Epoch& Epoch::operator-=(const Epoch& that)
    {
      itsDay -= that.itsDay;
      itsFrac -= that.itsFrac;
      adjust();
      return *this;
    }

    Epoch Epoch::operator-() const
    {
      return Epoch(-itsDay, -itsFrac);
    }

    //################  Private functions  ################//

    void Epoch::adjust()
    {
      while (itsFrac < 0) {
        itsFrac += 1;
        itsDay -= 1;
      }
      while (itsFrac >= 1) {
        itsFrac -= 1;
        itsDay += 1;
      }
    }


    void Epoch::add(double t)
    {
      double d = floor(t);
      itsDay += d;
      itsFrac += (t-d);
    }


    void Epoch::set(double t)
    {
      double d = floor(t);
      itsDay = d;
      itsFrac = (t-d);
    }


    //################  Free functions  ################//

    ostream& operator<< (ostream& os, const Epoch& time)
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


    //----------------  Comparison operators  ----------------//

    bool operator<(const Epoch& lhs, const Epoch& rhs)
    {
      return (lhs.getDay() < rhs.getDay() ||
              (lhs.getDay() == rhs.getDay() &&
               lhs.getFraction() < rhs.getFraction()));
    }

    bool operator>(const Epoch& lhs, const Epoch& rhs)
    {
      return rhs < lhs;
    }

    bool operator<=(const Epoch& lhs, const Epoch& rhs)
    {
      return !(rhs < lhs);
    }

    bool operator>=(const Epoch& lhs, const Epoch& rhs)
    {
      return !(lhs < rhs);
    }

    bool operator==(const Epoch& lhs, const Epoch& rhs)
    {
      return (lhs.getDay()      == rhs.getDay() &&
              lhs.getFraction() == rhs.getFraction());
    }

    bool operator!=(const Epoch& lhs, const Epoch& rhs)
    {
      return !(lhs == rhs);
    }


    //----------------  Numerical operators  ----------------//

    Epoch operator+(const Epoch& lhs, const Epoch& rhs)
    {
      return Epoch(lhs) += rhs;
    }

    Epoch operator-(const Epoch& lhs, const Epoch& rhs)
    {
      return Epoch(lhs) -= rhs;
    }

  } // namespace AMC

} // namespace LOFAR
