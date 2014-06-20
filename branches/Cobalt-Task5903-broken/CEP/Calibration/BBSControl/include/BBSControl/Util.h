//# Util.h: Miscellaneous utility functions.
//#
//# Copyright (C) 2012
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

#ifndef LOFAR_BBSCONTROL_UTIL_H
#define LOFAR_BBSCONTROL_UTIL_H

// \file
// Miscellaneous utility functions.

#include <BBSControl/Exceptions.h>
#include <ParmDB/Axis.h>
#include <Common/lofar_string.h>
#include <Common/lofar_sstream.h>
#include <Common/lofar_vector.h>
#include <utility>

#ifdef HAVE_PQXX
#include <BBSControl/CalSession.h>
#include <BBSControl/ProcessGroup.h>
#endif

namespace LOFAR
{
namespace BBS
{
using std::pair;

// \addtogroup BBSControl
// @{

#ifdef HAVE_PQXX
ProcessGroup makeProcessGroup(const CalSession &session);
#endif

// Try to convert to input string to the given type using an istringstream. An
// exception will be thrown if the conversion fails.
template <typename T>
T as(const string &in);

// Parse a range of unsigned integer values given as a string with a colon
// separator, e.g. "1024:2048".
pair<unsigned int, unsigned int> parseRange(const string &in);

// Parse a time range selection given as either one (start time) or two strings
// (start and end time). The range returned is the axis index range that
// falls within the time range, or an invalid range (1,0) if the selection and
// the axis do not overlap.
pair<size_t, size_t> parseTimeRange(const Axis::ShPtr &axis,
  const vector<string> &range);
// @}

template <typename T>
T as(const string &in)
{
  T out;
  istringstream iss(in);

  iss >> out;
  if(iss.fail() || iss.bad())
  {
    THROW(BBSControlException, "Conversion error: " << in);
  }
  return out;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
