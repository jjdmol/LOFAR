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

#ifndef LOFAR_BBS_BBSKERNEL_VISDATA_H
#define LOFAR_BBS_BBSKERNEL_VISDATA_H

#include <Common/LofarTypes.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_map.h>
#include <Common/lofar_set.h>
#include <Common/lofar_string.h>

#include <BBSKernel/Axis.h>

#include <boost/multi_array.hpp>
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

// TODO: Find an elegant way to unify VisGrid and VisData.

class VisGrid
{
public:
    cell_centered_axis<regular_series>      freq;
    cell_centered_axis<irregular_series>    time;
    set<baseline_t>                         baselines;
    set<string>                             polarizations;
};


class VisData
{
public:
    typedef shared_ptr<VisData>         Pointer;

    enum TimeslotFlag
    {
        UNAVAILABLE         = 1<<1,
        FLAGGED_IN_INPUT    = 1<<2,
        N_TimeslotFlag
    };

    VisData(uint32 nTimeslots,
        uint32 nBaselines,
        uint32 nChannels,
        uint32 nPolarizations);

    size_t getPolarizationCount() const
    { return polarizations.size(); }

    size_t getChannelCount() const
    { return freq.size(); }

    size_t getTimeslotCount() const
    { return time.size(); }

    size_t getBaselineCount() const
    { return baselines.size(); }

    bool hasBaseline(baseline_t baseline) const;
    size_t getBaselineIndex(baseline_t baseline) const;

    bool hasPolarization(const string &polarization) const;
    size_t getPolarizationIndex(const string &polarization) const;

    cell_centered_axis<regular_series>      freq;
    cell_centered_axis<irregular_series>    time;
    map<baseline_t, size_t>                 baselines;
    map<string, size_t>                     polarizations;

    boost::multi_array<double, 3>           uvw;
    boost::multi_array<tslot_flag_t, 2>     tslot_flag;
    boost::multi_array<flag_t, 4>           vis_flag;
    boost::multi_array<sample_t, 4>         vis_data;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
