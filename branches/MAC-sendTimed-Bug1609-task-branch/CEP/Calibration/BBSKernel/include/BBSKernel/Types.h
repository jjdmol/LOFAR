//# Types.h: Types used in the kernel.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_TYPES_H
#define LOFAR_BBSKERNEL_TYPES_H

#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarTypes.h>

#include <utility>

namespace LOFAR
{
namespace BBS
{
using std::pair;

// \addtogroup BBSKernel
// @{

// Type used to store visibility flags.
typedef uint8                   flag_t;
typedef pair<uint32, uint32>    baseline_t;

enum AxisType
{
    FREQ,
    TIME,
    N_AxisType
};

enum ParmCategory
{
    INSTRUMENT,
    SKY,
    N_ParmCategory
};

template <typename T_VALUE>
class Interval
{
public:
    Interval()
    {
    }

    Interval(T_VALUE start, T_VALUE end)
        :   start(start),
            end(end)
    {
    }

    T_VALUE start;
    T_VALUE end;
};

struct Vector3
{
    const double &operator[](size_t i) const
    { return __data[i]; }

    double &operator[](size_t i)
    { return __data[i]; }

    double  __data[3];
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
