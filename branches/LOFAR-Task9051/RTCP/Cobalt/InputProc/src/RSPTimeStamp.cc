//# RSPTimeStamp.cc: Small class to hold the timestamps from RSP
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "RSPTimeStamp.h"

#include <Common/lofar_iostream.h>
#include <Common/lofar_iomanip.h>

#include <CoInterface/TimeFuncs.h>

#include <sys/time.h>
#include <time.h>

namespace LOFAR
{
  namespace Cobalt
  {

    TimeStamp TimeStamp::now( unsigned clockSpeed )
    {
      struct timeval tv;

      gettimeofday(&tv, NULL);

      unsigned long usec = tv.tv_sec * 1000000 + tv.tv_usec;
      return convert((double)usec / 1000000, clockSpeed);
    }

    TimeStamp TimeStamp::universe_heat_death( unsigned clockSpeed )
    {
      return TimeStamp(0x7FFFFFFFFFFFFFFFUL, clockSpeed);
    }


    TimeStamp TimeStamp::convert( double seconds, unsigned clockSpeed )
    {
      return TimeStamp(seconds * clockSpeed / 1024, clockSpeed);
    }

    ostream &operator << (ostream &os, const TimeStamp &ts)
    {
      return os << TimeDouble::toString(ts.getSeconds(), true) << " UTC";
    }

  } // namespace Cobalt
} // namespace LOFAR

