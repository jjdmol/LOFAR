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

#ifndef COORD_TIMECOORD_H
#define COORD_TIMECOORD_H

//# Includes
#include <Common/lofar_iosfwd.h>

namespace LOFAR
{
  class TimeCoord
  {
  public:
    // Create from the current local date/time.
    TimeCoord();

    // Create from the given date and time.
    // Note that days, hours, minutes, and seconds can contain fractions.
    TimeCoord (int yy, int mm, double dd,
               double h=0, double m=0, double s=0);

    // Create from an MJD (with possible fractions of day for high accuracy).
    explicit TimeCoord (double mjd, double fraction=0);

    // Get the time in utc (in UNIX format in seconds).
    double utc() const;

    // Get the local time (in UNIX format in seconds).
    double local() const;

    // Get the time in MJD (as utc).
    double mjd() const
    { return itsDay + itsFrac; }

    // Get day and fraction.
    double getDay() const
    { return itsDay; }
    double getFraction() const
    { return itsFrac; }

    // Get year, month, day (possibly in local time).
    void ymd (int& yyyy, int& mm, int& dd, bool local=false) const;

    // Get hours, minutes seconds (possibly in local time).
    void hms (int& h, int& m, double& s, bool local=false) const;

    // Output in ASCII (in UTC).
    friend ostream& operator<< (ostream&, const TimeCoord&);

    // Return the difference between local time and UTC in seconds.
    // The difference is negative for time zones west of Greenwich.
    // So add this value to utc to get local time.
    static double getUTCDiff();

  private:
    double itsDay;     //# whole day in MJD
    double itsFrac;    //# Fraction of day
  };

} // namespace LOFAR

#endif
