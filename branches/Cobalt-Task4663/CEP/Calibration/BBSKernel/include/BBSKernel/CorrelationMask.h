//# CorrelationMask.h: Correlation selection implemented as a boolean mask.
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

#ifndef LOFAR_BBSKERNEL_CORRELATIONMASK_H
#define LOFAR_BBSKERNEL_CORRELATIONMASK_H

// \file
// Correlation selection implemented as a boolean mask. Provides fast access to
// the mask status of a correlation.

#include <BBSKernel/Correlation.h>
#include <BBSKernel/Exceptions.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class CorrelationMask
{
public:
    CorrelationMask(bool initial = false);

    // Mask correlation.
    void set(Correlation::Type correlation);
    // Unmask correlation.
    void clear(Correlation::Type correlation);

    // Get mask status of correlation.
    bool operator()(Correlation::Type correlation) const;

    // Returns true if nothing is masked. NB. This function is slow because
    // it needs to iterate over the entire mask. If this turns out to be a
    // bottleneck, the result could be cached internally.
    bool empty() const;

    friend const CorrelationMask operator!(const CorrelationMask &lhs);
    friend const CorrelationMask operator||(const CorrelationMask &lhs,
        const CorrelationMask &rhs);
    friend const CorrelationMask operator&&(const CorrelationMask &lhs,
        const CorrelationMask &rhs);

private:
    bool    itsMask[Correlation::N_Type];
};

// @}

// -------------------------------------------------------------------------- //
// - CorrelationMask implementation                                         - //
// -------------------------------------------------------------------------- //

inline void CorrelationMask::set(Correlation::Type correlation)
{
    if(!Correlation::isDefined(correlation))
    {
        THROW(BBSKernelException, "Attempt to mask an undefined correlation.");
    }
    itsMask[correlation] = true;
}

inline void CorrelationMask::clear(Correlation::Type correlation)
{
    if(!Correlation::isDefined(correlation))
    {
        THROW(BBSKernelException, "Attempt to unmask an undefined"
            " correlation.");
    }
    itsMask[correlation] = false;
}

inline bool CorrelationMask::operator()(Correlation::Type correlation) const
{
    return Correlation::isDefined(correlation) ? itsMask[correlation] : false;
}

} //# namespace BBS
} //# namespace LOFAR

#endif
