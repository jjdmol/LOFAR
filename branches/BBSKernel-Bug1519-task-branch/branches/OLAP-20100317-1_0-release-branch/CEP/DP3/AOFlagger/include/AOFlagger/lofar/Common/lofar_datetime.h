//# lofar_datetime.h: namespace wrapper for Boost.Date_Time Posix classes
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
//# $Id: lofar_datetime.h 14057 2009-09-18 12:26:29Z diepen $

#ifndef LOFAR_COMMON_DATETIME_H
#define LOFAR_COMMON_DATETIME_H

// \file
// namespace wrapper for Boost.Date_Time Posix classes

#if !defined(HAVE_BOOST)
#error Boost.Date_Time is required.
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include <time.h>

namespace LOFAR
{
  using namespace boost::posix_time;

	// there is no function in boost to convert a ptime to a time_t.
	inline time_t	to_time_t(ptime aPtime) {
		struct tm 	stm = to_tm(aPtime);
		return (mktime(&stm));
	}
}

#endif
