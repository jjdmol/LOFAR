//# VisData.cc: 
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
#include <BBSKernel/VisData.h>
#include <BBSKernel/Exceptions.h>

#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{

VisData::VisData(const VisDimensions &dims)
    :   uvw(boost::extents[dims.getBaselineCount()][dims.getTimeslotCount()][3]),
        tslot_flag(boost::extents[dims.getBaselineCount()]
            [dims.getTimeslotCount()]),
        vis_flag(boost::extents[dims.getBaselineCount()][dims.getTimeslotCount()]
            [dims.getChannelCount()][dims.getPolarizationCount()]),
        vis_data(boost::extents[dims.getBaselineCount()][dims.getTimeslotCount()]
            [dims.getChannelCount()][dims.getPolarizationCount()]),
        itsDimensions(dims)
{
    const size_t nChannels = dims.getChannelCount();
    const size_t nTimeslots = dims.getTimeslotCount();
    const size_t nBaselines = dims.getBaselineCount();
    const size_t nPolarizations = dims.getPolarizationCount();

    LOG_DEBUG_STR("Size: "
        << (nBaselines * nTimeslots * 3 * sizeof(double)
            + nBaselines * nTimeslots * sizeof(tslot_flag_t)
            + nBaselines * nTimeslots * nChannels * nPolarizations
                * sizeof(flag_t)
            + nBaselines * nTimeslots * nChannels * nPolarizations
                * sizeof(sample_t))
            / (1024.0 * 1024.0)
        << " Mb.");

    // Initially flag all timeslots as UNAVAILABLE.
    for(size_t i = 0; i < nBaselines; ++i)
    {
        for(size_t j = 0; j < nTimeslots; ++j)
        {
            tslot_flag[i][j] = VisData::UNAVAILABLE;
        }
    }
}


VisData::~VisData()
{
    LOG_DEBUG("VisData destructor called.");
}

} //# namespace BBS
} //# namespace LOFAR
