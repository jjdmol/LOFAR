//# Interval.h: A struct template that represents an interval. Similar to
//# std::pair<>, but the attribute names "start" and "end" make the code more
//# readable when dealing with intervals.
//#
//# Copyright (C) 2013
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

#ifndef LOFAR_BBSKERNEL_INTERVAL_H
#define LOFAR_BBSKERNEL_INTERVAL_H

// \file
// A struct template that represents an interval. Similar to std::pair<>, but
// the attribute names "start" and "end" make the code more readable when
// dealing with intervals.

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

template <typename T>
struct Interval
{
    Interval()
        :   start(),
            end()
    {
    }

    Interval(const T &start, const T &end)
        :   start(start),
            end(end)
    {
    }

    T   start, end;
};

template <typename T>
inline bool operator==(const Interval<T> &x, const Interval<T> &y)
{
    return x.start == y.start && x.end == y.end;
}

template <typename T>
inline bool operator!=(const Interval<T> &x, const Interval<T> &y)
{
    return !(x == y);
}

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
