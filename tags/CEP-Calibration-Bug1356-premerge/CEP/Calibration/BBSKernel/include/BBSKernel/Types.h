//# Types.h: Types used in the kernel.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

    typedef double                  real_t;
    typedef dcomplex                complex_t;
    typedef fcomplex                sample_t;
    typedef uint8                   flag_t;
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
