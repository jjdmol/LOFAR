//# CorrelationFilter.h: Filter used to select correlation products.
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

#ifndef LOFAR_BBSKERNEL_CORRELATIONFILTER_H
#define LOFAR_BBSKERNEL_CORRELATIONFILTER_H

// \file
// Filter used to select correlation products.

#include <Common/lofar_set.h>
#include <BBSKernel/Correlation.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class CorrelationFilter
{
public:
    // Check whether or not this filter will actually filter anything.
    bool empty() const;

    // Append a correlation selection criterion to the filter.
    void append(Correlation correlation);
    void append(const string &correlation);

    // Filter a list of correlations according to the filter.
    template <typename T_ITER, typename T_OUTPUT_ITER>
    void filter(T_ITER first, T_ITER last, T_OUTPUT_ITER out) const;

private:
    friend ostream &operator<<(ostream &out, const CorrelationFilter &obj);

    set<Correlation>    itsCorrelations;
};

// Write a CorrelationFilter instance to an output stream in human readable
// form.
ostream &operator<<(ostream &out, const CorrelationFilter &obj);

// @}

// -------------------------------------------------------------------------- //
// - CorrelationFilter implementation                                       - //
// -------------------------------------------------------------------------- //

template <typename T_ITER, typename T_OUTPUT_ITER>
void CorrelationFilter::filter(T_ITER first, T_ITER last, T_OUTPUT_ITER out)
    const
{
    for(; first != last; ++first)
    {
        if(itsCorrelations.empty()
            || itsCorrelations.find(*first) != itsCorrelations.end())
        {
            *out++ = *first;
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
