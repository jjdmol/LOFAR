//# VisBuffer.cc: A buffer of visibility data and associated information (e.g.
//# flags, UVW coordinates).
//#
//# Copyright (C) 2007
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

#include <lofar_config.h>
#include <BBSKernel/VisBuffer.h>
#include <BBSKernel/Exceptions.h>

#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS
{

VisBuffer::VisBuffer()
{
}

VisBuffer::VisBuffer(const VisDimensions &dims)
    :   uvw(boost::extents[dims.nBaselines()][dims.nTime()][3]),
        tslot_flag(boost::extents[dims.nBaselines()][dims.nTime()]),
        vis_flag(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()]),
        vis_data(boost::extents[dims.nBaselines()][dims.nTime()][dims.nFreq()]
            [dims.nCorrelations()]),
        itsDims(dims)
{
    const size_t nFreq = dims.nFreq();
    const size_t nTime = dims.nTime();
    const size_t nBaselines = dims.nBaselines();
    const size_t nCorrelations = dims.nCorrelations();

    LOG_DEBUG_STR("Size: "
        << (nBaselines * nTime * 3 * sizeof(double)
            + nBaselines * nTime * sizeof(tslot_flag_t)
            + nBaselines * nTime * nFreq * nCorrelations * sizeof(flag_t)
            + nBaselines * nTime * nFreq * nCorrelations * sizeof(sample_t))
            / (1024.0 * 1024.0)
        << " Mb.");

    // Initially flag all timeslots as UNAVAILABLE.
    for(size_t i = 0; i < nBaselines; ++i)
    {
        for(size_t j = 0; j < nTime; ++j)
        {
            tslot_flag[i][j] = VisBuffer::UNAVAILABLE;
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR
