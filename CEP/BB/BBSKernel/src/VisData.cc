//# VisData.cc: 
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

#include <lofar_config.h>
#include <BBSKernel/VisData.h>
#include <BBSKernel/Exceptions.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{

VisData::VisData(uint32 nTimeslots,
    uint32 nBaselines,
    uint32 nChannels,
    uint32 nPolarizations)
    :
    uvw(boost::extents[nBaselines][nTimeslots][3]),
    tslot_flag(boost::extents[nBaselines][nTimeslots]),
    vis_flag(boost::extents[nBaselines][nTimeslots][nChannels][nPolarizations]),
    vis_data(boost::extents[nBaselines][nTimeslots][nChannels][nPolarizations])
{
    LOG_DEBUG_STR("Chunk size: "
        << (nBaselines * nTimeslots * 3 * sizeof(double)
            + nBaselines * nTimeslots * sizeof(tslot_flag_t)
            + nBaselines * nTimeslots * nChannels * nPolarizations
                * sizeof(flag_t)
            + nBaselines * nTimeslots * nChannels * nPolarizations
                * sizeof(sample_t))
            / (1024.0 * 1024.0)
        << " Mb.");
}


bool VisData::hasBaseline(baseline_t baseline) const
{
    map<baseline_t, size_t>::const_iterator it = baselines.find(baseline);
    return it != baselines.end();
}


size_t VisData::getBaselineIndex(baseline_t baseline) const
{
    map<baseline_t, size_t>::const_iterator it = baselines.find(baseline);
    if(it != baselines.end())
        return it->second;
    else
        THROW(BBSKernelException, "Request for index of unknown baseline "
            << baseline.first << " - " << baseline.second);
}


bool VisData::hasPolarization(const string &polarization) const
{
    map<string, size_t>::const_iterator it = polarizations.find(polarization);
    return it != polarizations.end();
}


size_t VisData::getPolarizationIndex(const string &polarization) const
{
    map<string, size_t>::const_iterator it = polarizations.find(polarization);

    if(it != polarizations.end())
        return it->second;
    else
        THROW(BBSKernelException, "Request for index of unknown polarization "
            << polarization);
}

} //# namespace BBS
} //# namespace LOFAR
