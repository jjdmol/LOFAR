//# BaselineFilter.h: Filter used to select baselines by pattern matching on
//# station name(s) and baseline type. Shell style patterns (*,?,{}) are
//# supported.
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

#ifndef LOFAR_BBSKERNEL_BASELINEFILTER_H
#define LOFAR_BBSKERNEL_BASELINEFILTER_H

// \file
// Filter used to select baselines by pattern matching on station name(s) and
// baseline type. Shell style patterns (*,?,{}) are supported.

#include <Common/lofar_vector.h>
#include <BBSKernel/BaselineMask.h>
#include <BBSKernel/Instrument.h>

#include <casa/Utilities/Regex.h>

namespace LOFAR
{

// \addtogroup BBSKernel
// @{

// Write a pair of casa::Regex instances to an output stream in human readable
// form.
ostream &operator<<(ostream &out, const pair<casa::Regex, casa::Regex> &obj);

// @}

namespace BBS
{
// \addtogroup BBSKernel
// @{

class BaselineFilter
{
public:
    enum BaselineType
    {
        AUTO,
        CROSS,
        ANY,
        N_BaselineType
    };

    BaselineFilter();

    // Returns true if the filter contains no patterns.
    bool empty() const;

    // Set the baseline type (one of AUTO, CROSS, ANY).
    void setBaselineType(const string &type);
    void setBaselineType(BaselineType type);

    // Append a baseline selection pattern to the filter.
    void append(const string &patternLHS, const string &patternRHS = "*");

    // Get baseline type selection.
    BaselineType baselineType() const;

    // Create a baseline mask given a specific Instrument instance. The
    // Instrument instance contains the station names to match against.
    BaselineMask createMask(const Instrument &instrument) const;

    // Check if the BaselineType is valid (i.e. < N_BaselineType).
    static bool isDefined(BaselineType in);

    // Convert the input argument to the corresponding BaselineType. If the
    // input is out of bounds, N_BaselineType is returned.
    static BaselineType asBaselineType(unsigned int in);

    // Convert the input argument to the corresponding BaselineType. If the
    // input does not match any defined BaselineType, N_BaselineType is
    // returned.
    static BaselineType asBaselineType(const string &in);

    // Convert the input BaselineType to its string representation.
    // N_BaselineType converts to "<UNDEFINED>".
    static const string &asString(BaselineType in);

private:
    // Match a pattern against all the stations in the provided instrument
    // instance and write the indices of matching stations to the output
    // iterator.
    template <typename T_OUTPUT_ITER>
    void findMatchingStations(const Instrument &instrument,
        const casa::Regex &pattern, T_OUTPUT_ITER out) const;

    // Update a BaselineMask given two groups of station indices. All the
    // possible combinations between the two groups are masked.
    template <typename T_ITER>
    void update(BaselineMask &mask, T_ITER groupA, T_ITER groupA_end,
        T_ITER groupB, T_ITER groupB_end) const;

    friend ostream &operator<<(ostream &out, const BaselineFilter &obj);

    BaselineType                            itsBaselineType;
    vector<pair<casa::Regex, casa::Regex> > itsPatterns;
};

// Write a BaselineFilter instance to an output stream in human readable form.
ostream &operator<<(ostream &out, const BaselineFilter &obj);

// @}

// -------------------------------------------------------------------------- //
// - BaselineFilter implementation                                          - //
// -------------------------------------------------------------------------- //

inline BaselineFilter::BaselineType BaselineFilter::baselineType() const
{
    return itsBaselineType;
}

template <typename T_OUTPUT_ITER>
void BaselineFilter::findMatchingStations(const Instrument &instrument,
    const casa::Regex &pattern, T_OUTPUT_ITER out) const
{
    for(size_t i = 0; i < instrument.size(); ++i)
    {
        casa::String name(instrument[i].name());

        if(name.matches(pattern))
        {
            *out++ = i;
        }
    }
}

template <typename T_ITER>
void BaselineFilter::update(BaselineMask &mask, T_ITER groupA,
    T_ITER groupA_end, T_ITER groupB, T_ITER groupB_end) const
{
    for(T_ITER groupA_it = groupA; groupA_it != groupA_end; ++groupA_it)
    {
        for(T_ITER groupB_it = groupB; groupB_it != groupB_end; ++groupB_it)
        {
            if(itsBaselineType == ANY
                || (itsBaselineType == AUTO && *groupA_it == *groupB_it)
                || (itsBaselineType == CROSS && *groupA_it != *groupB_it))
            {
                mask.set(*groupA_it, *groupB_it);
                mask.set(*groupB_it, *groupA_it);
            }
        }
    }
}

} //# namespace BBS
} //# namespace LOFAR

#endif
