//# TimeFuncs.cc
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

#include <lofar_config.h>
#include "TimeFuncs.h"

#include <cmath>
#include <time.h>

namespace LOFAR
{
  namespace Cobalt
  {
    namespace TimeSpec
    {
      struct timespec now() {
        struct timespec ts;

#if _POSIX_C_SOURCE >= 199309L
#  ifdef CLOCK_REALTIME_COARSE
        clock_gettime(CLOCK_REALTIME_COARSE, &ts);
#  else
        clock_gettime(CLOCK_REALTIME, &ts);
#  endif
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);

        ts.tv_sec  = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec * 1000L;
#endif

        return ts;
      }


      double toDouble(const timespec &ts) {
        return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) / (1000.0 * 1000.0 * 1000.0);
      }


      void inc(struct timespec &ts, double seconds) {
        const long ns_per_second = 1000L * 1000L * 1000L;

        ts.tv_sec  += floor(seconds);
        ts.tv_nsec += (seconds - floor(seconds)) * ns_per_second;

        // normalize
        if (ts.tv_nsec >= ns_per_second) {
          ts.tv_nsec -= ns_per_second;
          ts.tv_sec++;
        }
      }


      double operator-(const timespec &end, const timespec &begin) {
        return toDouble(end) - toDouble(begin);
      }


      bool operator>(const struct timespec &a, const struct timespec &b) {
        return a.tv_sec > b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec > b.tv_nsec);
      }


      bool operator>=(const struct timespec &a, const struct timespec &b) {
        return a.tv_sec > b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec >= b.tv_nsec);
      }


      bool operator<(const struct timespec &a, const struct timespec &b) {
        return a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec < b.tv_nsec);
      }


      bool operator<=(const struct timespec &a, const struct timespec &b) {
        return a.tv_sec < b.tv_sec || (a.tv_sec == b.tv_sec && a.tv_nsec <= b.tv_nsec);
      }


      bool operator==(const struct timespec &a, const struct timespec &b) {
        return a.tv_sec == b.tv_sec && a.tv_nsec == b.tv_nsec;
      }


      bool operator!=(const struct timespec &a, const struct timespec &b) {
        return a.tv_sec != b.tv_sec || a.tv_nsec != b.tv_nsec;
      }

    }
  }
}

