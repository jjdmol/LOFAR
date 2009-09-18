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

    typedef fcomplex                sample_t;
    typedef bool                    flag_t;
    typedef uint8                   tslot_flag_t;
    typedef pair<uint32, uint32>    baseline_t;

    enum AxisName
    {
        FREQ,
        TIME,
        N_AxisName
    };
    
    enum ParmCategory
    {
        INSTRUMENT,
        SKY,
        N_ParmCategory
    };

} // namespace BBS
} // namespace LOFAR

#endif
