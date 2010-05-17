//# BaselineMask.h: Baseline selection implemented as a boolean mask. Provides
//# fast access to the mask status of a baseline.
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
// Baseline selection implemented as a boolean mask. Provides fast access to the
// mask status of a baseline.

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
    // Mask baseline.
    void set(const baseline_t &baseline);
    // Unmask baseline.
    void clear(const baseline_t &baseline);

    // Mask baseline (i, j) (convenience function).
    void set(size_t i, size_t j);
    // Unmask baseline (i, j) (convenience function).
    void clear(size_t i, size_t j);

    // Get mask status of baseline.
    bool operator()(const baseline_t &baseline) const;
    // Get mask status of baseline (i, j) (convenience function).
    bool operator()(size_t i, size_t j) const;

    // Returns true if nothing is masked. NB. This function is slow because
    // it needs to iterate over the entire mask. If this turns out to be a
    // bottleneck, the result could be cached internally.
    bool empty() const;

    // Mask arithmetic.
    friend const BaselineMask operator!(const BaselineMask &lhs);
    friend const BaselineMask operator||(const BaselineMask &lhs,
        const BaselineMask &rhs);
    friend const BaselineMask operator&&(const BaselineMask &lhs,
        const BaselineMask &rhs);

private:
    // Compute the linear index of baseline (i, j).
    size_t index(size_t i, size_t j) const;

    vector<bool>    itsMask;
};

// @}

// -------------------------------------------------------------------------- //
// - BaselineMask implementation                                            - //
// -------------------------------------------------------------------------- //

inline void BaselineMask::set(const baseline_t &baseline)
{
    set(baseline.first, baseline.second);
}

inline void BaselineMask::clear(const baseline_t &baseline)
{
    clear(baseline.first, baseline.second);
}

inline void BaselineMask::set(size_t i, size_t j)
{
    const size_t index = this->index(i, j);
    if(index >= itsMask.size())
    {
        itsMask.resize(index + 1);
    }

    itsMask[index] = true;
}

inline void BaselineMask::clear(size_t i, size_t j)
{
    const size_t index = this->index(i, j);
    if(index < itsMask.size())
    {
        itsMask[index] = false;
    }
}

inline bool BaselineMask::operator()(const baseline_t &baseline) const
{
    return this->operator()(baseline.first, baseline.second);
}

inline bool BaselineMask::operator()(size_t i, size_t j) const
{
    const size_t index = this->index(i, j);
    return index < itsMask.size() && itsMask[index];
}

inline size_t BaselineMask::index(size_t i, size_t j) const
{
    //# Compute the diagonal that contains (i, j).
    const size_t d = i + j;
    //# NB. Rounding down because of integer division (intended).
    const size_t d_2 = d / 2;

    return d_2 * d_2 + d_2 + (d % 2) * d_2 + (d % 2) + std::min(i, j);
}

} //# namespace BBS
} //# namespace LOFAR

#endif
