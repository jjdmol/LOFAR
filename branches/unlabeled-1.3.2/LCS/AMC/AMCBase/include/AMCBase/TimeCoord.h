//# TimeCoord.h: Class to hold a time coordinate as 2 values
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

#ifndef LOFAR_AMCBASE_TIMECOORD_H
#define LOFAR_AMCBASE_TIMECOORD_H

//# Forward Declarations.
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  namespace AMC
  {

    class TimeCoord
    {
    public:
      // Create from the current local date/time.
      TimeCoord();

      // Create from the given UTC date and time.
      // Note that days, hours, minutes, and seconds can contain fractions.
      TimeCoord (int yy, int mm, double dd,
                 double h=0, double m=0, double s=0);

      // Create from an MJD (with possible fractions of day for high accuracy).
      explicit TimeCoord (double mjd, double fraction=0);

      // @{
      // Get/set the UTC time in seconds since January 1, 1970 (Unix format).
      double utc() const;
      void utc(double s);
      // @}

      // @{
      // Get/set the local time in seconds since January 1, 1970 (Unix format).
      double local() const;
      void local(double s);
      // @}

      // @{
      // Get/set the UTC time in MJD.
      double mjd() const;
      void mjd(double mjd);
      // @}

      // @{
      // Get day and fraction.
      double getDay() const;
      double getFraction() const;
      // @}

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

    private:
      double itsDay;     //# whole day in MJD
      double itsFrac;    //# Fraction of day
    };

    // Output in ASCII (in UTC).
    ostream& operator<< (ostream&, const TimeCoord&);


    //######################## INLINE FUNCTIONS ########################//

    inline double TimeCoord::utc() const
    {
      return (mjd() - 40587) * 24 * 3600;
    }

    inline double TimeCoord::local() const
    {
      return utc() + getUTCDiff();
    }

    inline double TimeCoord::mjd() const
    {
      return itsDay + itsFrac; 
    }

    inline double TimeCoord::getDay() const
    {
      return itsDay;
    }

    inline double TimeCoord::getFraction() const
    {
      return itsFrac;
    }


  } // namespace AMC

} // namespace LOFAR

#endif
