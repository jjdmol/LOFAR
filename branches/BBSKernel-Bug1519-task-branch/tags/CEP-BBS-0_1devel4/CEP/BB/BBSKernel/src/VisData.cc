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

#include <Common/lofar_algorithm.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
namespace BBS 
{

VisData::VisData(const VisGrid &visGrid)
    :   grid(visGrid),
        uvw(boost::extents[grid.baselines.size()][grid.time.size()][3]),
        tslot_flag(boost::extents[grid.baselines.size()][grid.time.size()]),
        vis_flag(boost::extents[grid.baselines.size()][grid.time.size()]
            [grid.freq.size()][grid.polarizations.size()]),
        vis_data(boost::extents[grid.baselines.size()][grid.time.size()]
            [grid.freq.size()][grid.polarizations.size()])
{
    size_t nTimeslots = grid.time.size();
    size_t nBaselines = grid.baselines.size();
    size_t nChannels = grid.freq.size();
    size_t nPolarizations = grid.polarizations.size();

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
        for(size_t j = 0; j < nTimeslots; ++j)
    {
        tslot_flag[i][j] = VisData::UNAVAILABLE;
    }

    size_t index;

    // Initialize baseline index.
    index = 0;
    for(vector<baseline_t>::const_iterator it = grid.baselines.begin(),
        end = grid.baselines.end();
        it != end;
        ++it)
    {
        baseline_idx[*it] = index;
        ++index;
    }

    // Initialize polarizations index.
    index = 0;
    for(vector<string>::const_iterator it = grid.polarizations.begin(),
        end = grid.polarizations.end();
        it != end;
        ++it)
    {
        polarization_idx[*it] = index;
        ++index;
    }
}


VisData::~VisData()
{
    LOG_DEBUG("VisData destructor called.");
}


bool VisData::hasBaseline(baseline_t baseline) const
{
    map<baseline_t, size_t>::const_iterator it = baseline_idx.find(baseline);
    return it != baseline_idx.end();
}


size_t VisData::getBaselineIndex(baseline_t baseline) const
{
    map<baseline_t, size_t>::const_iterator it = baseline_idx.find(baseline);
    if(it != baseline_idx.end())
    {
        return it->second;
    }
    THROW(BBSKernelException, "Request for index of unknown baseline "
        << baseline.first << " - " << baseline.second);
}


bool VisData::hasPolarization(const string &polarization) const
{
    map<string, size_t>::const_iterator it = polarization_idx.find(polarization);
    return it != polarization_idx.end();
}


size_t VisData::getPolarizationIndex(const string &polarization) const
{
    map<string, size_t>::const_iterator it = polarization_idx.find(polarization);
    if(it != polarization_idx.end())
    {
        return it->second;
    }
    THROW(BBSKernelException, "Request for index of unknown polarization "
        << polarization);
}

} //# namespace BBS
} //# namespace LOFAR
