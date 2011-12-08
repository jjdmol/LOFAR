//# UVWFlagger.h: Flag visibility samples based on baseline (UVW) length.
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

#ifndef LOFAR_BBSKERNEL_UVWFLAGGER_H
#define LOFAR_BBSKERNEL_UVWFLAGGER_H

// \file
// Flag visibilities based on various criteria.

#include <Common/Timer.h>
#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/Types.h>
#include <BBSKernel/VisBuffer.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class UVWFlagger
{
public:
    UVWFlagger(const VisBuffer::Ptr &buffer);

    // Set bitmask that will be OR-ed with the flags of a sample that exceeds
    // the UVW threshold.
    void setFlagMask(flag_t mask);

    // Set minimal UV distance in wavelengths.
    void setUVMin(double min);
    // Set maximal UV distance in wavelengths.
    void setUVMax(double max);
    // Set UV interval in wavelengths.
    void setUVRange(double min, double max);

    // Restrict processing to the baselines included in the mask.
    void setBaselineMask(const BaselineMask &mask);

    // Is the current visibility selection empty? The visibility selection is
    // determined by the selected baselines and correlations.
    bool isSelectionEmpty() const;

    // Flag the visibility data.
    void process();

    // Reset processing statistics.
    void clearStats();

    // Dump processing statistics to the provided output stream.
    void dumpStats(ostream &out) const;

private:
    VisBuffer::Ptr      itsBuffer;
    BaselineMask        itsBaselineMask;

    flag_t              itsMask;
    Interval<double>    itsUVIntervalSqr;

    NSTimer             itsTimer;
    size_t              itsFlagCount, itsSampleCount;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
