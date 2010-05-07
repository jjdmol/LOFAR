//# BaselineFilter.cc: Filter used to select baselines by pattern matching on
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

#include <lofar_config.h>

#include <Common/StreamUtil.h>
#include <BBSKernel/BaselineFilter.h>
#include <BBSKernel/Exceptions.h>

namespace LOFAR
{

// Needs to be defined inside the LOFAR namespace because operator<< for
// vector<T> (defined in Common/StreamUtil.h) is defined there and therefore
// will not find an operator<< defined in LOFAR::BBS.
ostream &operator<<(ostream &out, const pair<casa::Regex, casa::Regex> &obj)
{
    out << "(" << obj.first.regexp() << "," << obj.second.regexp() << ")";
    return out;
}

namespace BBS
{
using LOFAR::operator<<;

BaselineFilter::BaselineFilter()
    : itsBaselineType(ANY)
{
}

bool BaselineFilter::empty() const
{
    return itsPatterns.empty();
}

void BaselineFilter::setBaselineType(const string &type)
{
    try
    {
        setBaselineType(BaselineFilter::asBaselineType(type));
    }
    catch(Exception &ex)
    {
        THROW(BBSKernelException, "Undefined baseline type: " << type);
    }
}

void BaselineFilter::setBaselineType(BaselineFilter::BaselineType type)
{
    ASSERT(BaselineFilter::isDefined(type));
    itsBaselineType = type;
}

void BaselineFilter::append(const string &patternLHS, const string &patternRHS)
{
    try
    {
        itsPatterns.push_back(make_pair(casa::Regex::fromPattern(patternLHS),
            casa::Regex::fromPattern(patternRHS)));
    }
    catch(casa::AipsError &ex)
    {
        THROW(BBSKernelException, "Error parsing baseline pattern: ("
            << patternLHS << "," << patternRHS << ") (exception: " << ex.what()
            << ")");
    }
}

BaselineMask BaselineFilter::createMask(const Instrument &instrument) const
{
    // TODO: If this method turns out to be a bottleneck it should not be to
    // difficult to optimize it.
    BaselineMask mask;

    // Process baseline patterns.
    typedef vector<pair<casa::Regex, casa::Regex> >::const_iterator
        PatternIter;

    for(PatternIter pattern = itsPatterns.begin(),
        pattern_end = itsPatterns.end(); pattern != pattern_end; ++pattern)
    {
        vector<size_t> groupA, groupB;

        findMatchingStations(instrument, pattern->first,
            back_inserter(groupA));
        findMatchingStations(instrument, pattern->second,
            back_inserter(groupB));
        update(mask, groupA.begin(), groupA.end(), groupB.begin(),
            groupB.end());
    }

    return mask;
}

bool BaselineFilter::isDefined(BaselineType in)
{
    return in != N_BaselineType;
}

BaselineFilter::BaselineType BaselineFilter::asBaselineType(unsigned int in)
{
    BaselineType out = N_BaselineType;
    if(in < N_BaselineType)
    {
        out = static_cast<BaselineType>(in);
    }

    return out;
}

BaselineFilter::BaselineType BaselineFilter::asBaselineType(const string &in)
{
    BaselineType out = N_BaselineType;
    for(unsigned int i = 0; i < N_BaselineType; ++i)
    {
        if(in == asString(static_cast<BaselineType>(i)))
        {
            out = static_cast<BaselineType>(i);
            break;
        }
    }

    return out;
}

const string &BaselineFilter::asString(BaselineFilter::BaselineType in)
{
    //# Caution: Always keep this array of strings in sync with the enum
    //# BaselineType that is defined in the header.
    static const string blType[N_BaselineType + 1] =
        {"AUTO",
        "CROSS",
        "ANY",
        //# "<UNDEFINED>" should always be last.
        "<UNDEFINED>"};

    return blType[in];
}

ostream &operator<<(ostream &out, const BaselineFilter &obj)
{
    out << "BaselineFilter:";

    Indent id;
    out << endl << indent << "Type: "
        << BaselineFilter::asString(obj.baselineType());
    out << endl << indent << "Baseline pattern(s): " << obj.itsPatterns;

    return out;
}

} //# namespace BBS
} //# namespace LOFAR
