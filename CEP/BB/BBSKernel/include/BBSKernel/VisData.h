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

#include <BBSKernel/Axis.h>

#include <Common/LofarTypes.h>
#include <stddef.h>

#include <Common/lofar_smartptr.h>
#include <Common/lofar_map.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <utility>
#include <boost/multi_array.hpp>

namespace LOFAR
{
namespace BBS
{
using std::pair;

typedef fcomplex                sample_t;
typedef bool                    flag_t;
typedef uint8                   tslot_flag_t;
typedef pair<uint32, uint32>    baseline_t;


class VisGrid
{
public:
    cell_centered_axis<regular_series>      freq;
    cell_centered_axis<irregular_series>    time;
    vector<baseline_t>                      baselines;
    vector<string>                          polarizations;
};


class VisData
{
public:
    typedef shared_ptr<VisData>     Pointer;

    enum TimeslotFlag
    {
        UNAVAILABLE         = 1<<1,
        FLAGGED_IN_INPUT    = 1<<2,
        N_TimeslotFlag
    };

    VisData(const VisGrid &visGrid);
    ~VisData();

    bool hasBaseline(baseline_t baseline) const;
    size_t getBaselineIndex(baseline_t baseline) const;

    bool hasPolarization(const string &polarization) const;
    size_t getPolarizationIndex(const string &polarization) const;

    // Grid on which the visibility data is sampled.
    VisGrid                                 grid;

    // Data
    boost::multi_array<double, 3>           uvw;
    boost::multi_array<tslot_flag_t, 2>     tslot_flag;
    boost::multi_array<flag_t, 4>           vis_flag;
    boost::multi_array<sample_t, 4>         vis_data;

private:
    // Indexes
    map<baseline_t, size_t>                 baseline_idx;
    map<string, size_t>                     polarization_idx;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
