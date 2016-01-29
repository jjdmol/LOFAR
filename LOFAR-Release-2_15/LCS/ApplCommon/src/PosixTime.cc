//# PosixTime.h: conversion routines from/to Unix to/from Posix time.
//#
//# Copyright (C) 2002-2012
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
//# $Id: PosixTime.h 22959 2012-11-23 11:09:22Z loose $

#include <lofar_config.h>
#include <ApplCommon/PosixTime.h>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace LOFAR
{
  using namespace boost;

  time_t to_time_t(posix_time::ptime aPtime)
  {
    posix_time::ptime epoch(gregorian::date(1970, 1, 1));
    posix_time::time_duration diff(aPtime - epoch);
    return diff.total_seconds();
  }

  posix_time::ptime from_ustime_t(double secsEpoch1970) 
  {
    time_t sec(static_cast<time_t>(secsEpoch1970));
    long usec(static_cast<long>(1000000 * (secsEpoch1970 - sec)));
    return posix_time::from_time_t(sec) + posix_time::microseconds(usec);
	}

}

