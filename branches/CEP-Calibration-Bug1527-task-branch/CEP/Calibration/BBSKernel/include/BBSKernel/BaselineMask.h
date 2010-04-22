//# BaselineMask.h: Baseline selection in the form of a boolean baseline mask.
//# Can decide if a given baseline is selected in O(1) time.
//#
//# Copyright (C) 2010
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

#ifndef LOFAR_BBSKERNEL_BASELINEMASK_H
#define LOFAR_BBSKERNEL_BASELINEMASK_H

// \file
// Baseline selection in the form of a boolean baseline mask. Can decide if a
// given baseline is selected in O(1) time.

#include <Common/lofar_vector.h>
#include <BBSKernel/Types.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class BaselineMask
{
public:
    explicit BaselineMask(size_t nStations = 0, bool initial = false);

    template <typename T_ITER>
    BaselineMask(size_t nStations, T_ITER first, T_ITER last);

    // Mask all baselines including station i.
    void mask(size_t i);
    // Mask baseline (i, j).
    void mask(size_t i, size_t j);
    // Mask baseline.
    void mask(const baseline_t &baseline);

    // Unmask all baselines including station i.
    void unmask(size_t i);
    // Unmask baseline (i, j).
    void unmask(size_t i, size_t j);
    // Unmask baseline.
    void unmask(const baseline_t &baseline);

    // Number of stations.
    size_t nStations() const;

    // Get mask status of station i.
    bool operator()(size_t i) const;
    // Get mask status of baseline (i, j).
    bool operator()(size_t i, size_t j) const;
    // Get mask status of baseline.
    bool operator()(const baseline_t &bl) const;

    // Filter a list of station indices or baselines according to the mask.
    template <typename T_ITER, typename T_OUTPUT_ITER>
    void filter(T_ITER first, T_ITER last, T_OUTPUT_ITER out) const;

private:
    // Update the station mask (only performed when necessary as it can be an
    // expensive operation).
    void updateStationMask() const;

    mutable bool            itsUpdateStationMask;
    mutable vector<bool>    itsStationMask;
    vector<bool>            itsMask;
};

// @}

// -------------------------------------------------------------------------- //
// - BaselineMask implementation                                            - //
// -------------------------------------------------------------------------- //

template <typename T_ITER>
BaselineMask::BaselineMask(size_t nStations, T_ITER first, T_ITER last)
    :   itsUpdateStationMask(false),
        itsStationMask(nStations, false),
        itsMask(nStations * nStations, false)
{
    for(; first != last; ++first)
    {
        mask(*first);
    }
}

inline size_t BaselineMask::nStations() const
{
    return itsStationMask.size();
}

inline bool BaselineMask::operator()(size_t i) const
{
    if(itsUpdateStationMask)
    {
        updateStationMask();
    }

    return itsStationMask[i];
}

inline bool BaselineMask::operator()(size_t i, size_t j) const
{
    return itsMask[i * nStations() + j];
}

inline bool BaselineMask::operator()(const baseline_t &bl) const
{
    return this->operator()(bl.first, bl.second);
}

template <typename T_ITER, typename T_OUTPUT_ITER>
void BaselineMask::filter(T_ITER first, T_ITER last, T_OUTPUT_ITER out) const
{
    for(; first != last; ++first)
    {
        if(this->operator()(*first))
        {
            *out++ = *first;
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
