//# Epoch.h: Class to hold a time coordinate as 2 values
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_AMCBASE_TIMECOORD_H
#define LOFAR_AMCBASE_TIMECOORD_H

// \file
// Class to hold a time coordinate as 2 values

//# Forward Declarations.
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace AMC
  {

    // \addtogroup AMCBase
    // @{

    // This class represents a moment in time. The time is stored internally
    // using two doubles: the first representing days as Modified Julian Day
    // (MJD); the second representing a fraction of a day. 
    class Epoch
    {
    public:
      // Create from the current local date/time.
      Epoch();

      // Create from the given UTC date and time.
      // Note that days, hours, minutes, and seconds can contain fractions.
      Epoch (int yy, int mm, double dd,
                 double h=0, double m=0, double s=0);

      // Create from an MJD (with possible fractions of day for high accuracy).
      Epoch (double mjd, double fraction=0);

      // Get the UTC time in seconds since January 1, 1970 (Unix format).
      double utc() const
      { return (mjd() - 40587) * 24 * 3600; }

      // Set the UTC time in seconds since January 1, 1970 (Unix format).
      void utc(double s);

      // Get the local time in seconds since January 1, 1970 (Unix format).
      double local() const
      { return utc() + getUTCDiff(); }

      // Set the local time in seconds since January 1, 1970 (Unix format).
      void local(double s);

      // Get the UTC time in MJD.
      double mjd() const
      { return itsDay + itsFrac; }

      // Set the UTC time in MJD.
      void mjd(double mjd)
      { set(mjd); }

      // Get day as Modified Julian Day (MJD).
      double getDay() const
      { return itsDay; }

      // Get fraction of the day.
      double getFraction() const
      { return itsFrac; }

      // Get year, month, day. If \a local is false, return UTC time; else
      // return local time.
      void ymd (int& yyyy, int& mm, int& dd, bool local=false) const;

      // Get hours, minutes seconds. If \a local is false, return UTC time;
      // else return local time.
      void hms (int& h, int& m, double& s, bool local=false) const;

      // Return the difference between local time and UTC in seconds.
      // The difference is negative for time zones west of Greenwich.
      // So add this value to utc to get local time.
      static double getUTCDiff();

      // Add the epoch \a that to \c this.
      Epoch& operator+=(const Epoch& that);

      // Subtract the epoch \a that from \c this.
      Epoch& operator-=(const Epoch& that);

      // Return the negation of \c this.
      Epoch operator-() const;

    private:
      double itsDay;     //# whole day in MJD
      double itsFrac;    //# Fraction of day

      // Adjust itsDay and itsFrac such that 0 <= itsFrac < 1.
      void adjust();

      // Add \a t days to the current Epoch object.
      void add(double t);

      // Set the current Epoch object to \a t days.
      void set(double t);

    };

    // Output in ASCII (in UTC).
    ostream& operator<< (ostream&, const Epoch&);

    // @name Comparison operators
    // @{
    bool operator<(const Epoch& lhs, const Epoch& rhs);
    bool operator>(const Epoch& lhs, const Epoch& rhs);
    bool operator<=(const Epoch& lhs, const Epoch& rhs);
    bool operator>=(const Epoch& lhs, const Epoch& rhs);
    bool operator==(const Epoch& lhs, const Epoch& rhs);
    bool operator!=(const Epoch& lhs, const Epoch& rhs);
    // @}

    // Return the sum of the epochs \a lhs and \a rhs.
    Epoch operator+(const Epoch& lhs, const Epoch& rhs);

    // Return the difference between the epochs \a lhs and \a rhs.
    Epoch operator-(const Epoch& lhs, const Epoch& rhs);

    // @}

  } // namespace AMC

} // namespace LOFAR

#endif
