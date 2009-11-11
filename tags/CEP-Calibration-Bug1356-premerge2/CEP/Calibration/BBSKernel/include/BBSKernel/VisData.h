//# VisData.h:
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSKERNEL_VISDATA_H
#define LOFAR_BBSKERNEL_VISDATA_H

#include <Common/lofar_smartptr.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <stddef.h>
#include <utility>
#include <boost/multi_array.hpp>

#include <BBSKernel/Types.h>
#include <BBSKernel/VisDimensions.h>

namespace LOFAR
{
namespace BBS
{
using std::pair;

// \addtogroup BBSKernel
// @{

class VisData
{
public:
    typedef shared_ptr<VisData>         Ptr;
    typedef shared_ptr<const VisData>   ConstPtr;

    enum TimeslotFlag
    {
        UNAVAILABLE         = 1<<1,
        FLAGGED_IN_INPUT    = 1<<2,
        N_TimeslotFlag
    };

    VisData(const VisDimensions &dims);
    ~VisData();

    const VisDimensions &getDimensions() const
    { return itsDimensions; }

    // Data
    boost::multi_array<double, 3>           uvw;
    boost::multi_array<tslot_flag_t, 2>     tslot_flag;
    boost::multi_array<flag_t, 4>           vis_flag;
    boost::multi_array<sample_t, 4>         vis_data;

private:
    // Description of the four dimensions (freq, time, baseline, polarization).
    VisDimensions                           itsDimensions;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
