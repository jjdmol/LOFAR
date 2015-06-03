//# BaselineMask.cc: Baseline selection implemented as a boolean mask. Provides
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

#include <lofar_config.h>
#include <BBSKernel/BaselineMask.h>
#include <Common/lofar_algorithm.h>

namespace LOFAR
{
namespace BBS
{

BaselineMask::BaselineMask(bool defaultMask)
    :   itsDefaultMask(defaultMask)
{
}

bool BaselineMask::empty() const
{
    return itsDefaultMask || count(itsMask.begin(), itsMask.end(), true) == 0;
}

const BaselineMask operator!(const BaselineMask &lhs)
{
    BaselineMask mask(!lhs.itsDefaultMask);

    const size_t size = lhs.itsMask.size();
    mask.itsMask.reserve(size);

    for(size_t i = 0; i < size; ++i)
    {
        mask.itsMask.push_back(!lhs.itsMask[i]);
    }

    return mask;
}

const BaselineMask operator||(const BaselineMask &lhs, const BaselineMask &rhs)
{
    BaselineMask mask(lhs.itsDefaultMask || rhs.itsDefaultMask);

    const size_t size = std::max(lhs.itsMask.size(), rhs.itsMask.size());
    mask.itsMask.reserve(size);

    for(size_t i = 0; i < size; ++i)
    {
        mask.itsMask.push_back(lhs.itsMask[i] || rhs.itsMask[i]);
    }

    return mask;
}

const BaselineMask operator&&(const BaselineMask &lhs, const BaselineMask &rhs)
{
    BaselineMask mask(lhs.itsDefaultMask && rhs.itsDefaultMask);

    size_t size = std::max(lhs.itsMask.size(), rhs.itsMask.size());
    mask.itsMask.reserve(size);

    for(size_t i = 0; i < size; ++i)
    {
        mask.itsMask.push_back(lhs.itsMask[i] && rhs.itsMask[i]);
    }

    return mask;
}

} //# namespace BBS
} //# namespace LOFAR
