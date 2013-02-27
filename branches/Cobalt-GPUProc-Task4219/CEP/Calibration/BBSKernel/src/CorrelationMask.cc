//# CorrelationMask.cc: Correlation selection implemented as a boolean mask.
//# Provides fast access to the mask status of a correlation.
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
#include <BBSKernel/CorrelationMask.h>
#include <Common/lofar_algorithm.h>

namespace LOFAR
{
namespace BBS
{

CorrelationMask::CorrelationMask(bool initial)
{
    fill(itsMask, itsMask + Correlation::N_Type, initial);
}

bool CorrelationMask::empty() const
{
    return count(itsMask, itsMask + Correlation::N_Type, true) == 0;
}

const CorrelationMask operator!(const CorrelationMask &lhs)
{
    CorrelationMask mask;
    for(size_t i = 0; i < Correlation::N_Type; ++i)
    {
        mask.itsMask[i] = !lhs.itsMask[i];
    }
    return mask;
}

const CorrelationMask operator||(const CorrelationMask &lhs,
    const CorrelationMask &rhs)
{
    CorrelationMask mask;
    for(size_t i = 0; i < Correlation::N_Type; ++i)
    {
        mask.itsMask[i] = lhs.itsMask[i] || rhs.itsMask[i];
    }
    return mask;
}

const CorrelationMask operator&&(const CorrelationMask &lhs,
    const CorrelationMask &rhs)
{
    CorrelationMask mask;
    for(size_t i = 0; i < Correlation::N_Type; ++i)
    {
        mask.itsMask[i] = lhs.itsMask[i] && rhs.itsMask[i];
    }
    return mask;
}

} //# namespace BBS
} //# namespace LOFAR
