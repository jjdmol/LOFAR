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
//# $Id$

#ifndef APPLCOMMON_POSIXTIME_H
#define APPLCOMMON_POSIXTIME_H

// \file
// Conversion routines from/to Unix to/from Posix time.

#if !defined(HAVE_BOOST_DATE_TIME)
#error Boost.Date_Time is required.
#endif

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LOFAR
{
  using namespace boost::posix_time;

	inline time_t to_time_t(ptime aPtime) {
          ptime epoch(boost::gregorian::date(1970, 1, 1));
          time_duration diff(aPtime - epoch);
          return diff.total_seconds();
	}

	inline ptime from_ustime_t(double	secsEpoch1970) {
		return (from_time_t((time_t)trunc(secsEpoch1970)) + microseconds((int64_t)((secsEpoch1970-trunc(secsEpoch1970))*1000000)));
	}
}

#endif
