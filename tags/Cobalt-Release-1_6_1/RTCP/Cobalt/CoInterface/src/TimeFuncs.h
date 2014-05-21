//# TimeFuncs.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifndef LOFAR_COINTERFACE_TIMEFUNCS_H
#define LOFAR_COINTERFACE_TIMEFUNCS_H

#include <ctime>
#include <climits>
#include <sys/time.h>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace TimeSpec
    {
      // A timestamp earlier than any other.
      const static struct timespec big_bang = { 0, 0 };

      // A timestamp later than any other.
      const static struct timespec universe_heat_death = { LONG_MAX, 999999999 };

      // Returns the current time, as a struct timespec
      struct timespec now();

      // Converts a timespec to double
      double toDouble(const struct timespec &ts);

      // Increment a timespec with a certain number of seconds
      void inc(struct timespec &ts, double seconds);

      // Return end - begin
      double operator-(const struct timespec &end, const struct timespec &begin);

      // Returns whether a is later than b
      bool operator>(const struct timespec &a, const struct timespec &b);

      // Provides (in)equality for timespecs
      bool operator==(const struct timespec &a, const struct timespec &b);
      bool operator!=(const struct timespec &a, const struct timespec &b);
    }
  }
}

#endif

