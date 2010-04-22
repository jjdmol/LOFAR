//# VisSelection.cc: Selection of visibility data that can exist independent of
//# a specific measurement (e.g. baselines are specified by name and not by a
//# pair of indices).
//#
//# Copyright (C) 2007
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
#include <BBSKernel/VisSelection.h>
#include <Common/lofar_algorithm.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>


namespace LOFAR
{
namespace BBS
{
using casa::Quantum;
using casa::MVTime;
using casa::Double;

VisSelection::VisSelection()
{
    fill(itsFlags, itsFlags + N_Field, false);
}

void VisSelection::clear(Field field)
{
    itsFlags[field] = false;
}

bool VisSelection::isSet(Field field) const
{
    return itsFlags[field];
}

bool VisSelection::empty() const
{
    return count(itsFlags, itsFlags + N_Field, true) == 0;
}

pair<size_t, size_t> VisSelection::getChannelRange() const
{
    return itsChannelRange;
}

pair<double, double> VisSelection::getTimeRange() const
{
    return itsTimeRange;
}

void VisSelection::setStartChannel(size_t start)
{
    if(!isSet(CHANNEL_END) || start <= itsChannelRange.second)
    {
        itsFlags[CHANNEL_START] = true;
        itsChannelRange.first = start;
    }
}

void VisSelection::setEndChannel(size_t end)
{
    if(!isSet(CHANNEL_START) || end >= itsChannelRange.first)
    {
        itsFlags[CHANNEL_END] = true;
        itsChannelRange.second = end;
    }
}

void VisSelection::setChannelRange(size_t start, size_t end)
{
    setStartChannel(start);
    setEndChannel(end);
}

void VisSelection::setStartTime(double start)
{
    if(!isSet(TIME_END) || start <= itsTimeRange.second)
    {
        itsFlags[TIME_START] = true;
        itsTimeRange.first = start;
    }
}

void VisSelection::setEndTime(double end)
{
    if(!isSet(TIME_START) || end >= itsTimeRange.first)
    {
        itsFlags[TIME_END] = true;
        itsTimeRange.second = end;
    }
}

void VisSelection::setStartTime(const string &start)
{
    double time;
    if(convertTime(start, time))
    {
        setStartTime(time);
    }
}

void VisSelection::setEndTime(const string &end)
{
    double time;
    if(convertTime(end, time))
    {
        setEndTime(time);
    }
}

void VisSelection::setTimeRange(double start, double end)
{
    setStartTime(start);
    setEndTime(end);
}

void VisSelection::setTimeRange(const string &start, const string &end)
{
    setStartTime(start);
    setEndTime(end);
}

void VisSelection::setBaselineFilter(const BaselineFilter &filter)
{
    if(!filter.empty())
    {
        itsFlags[BASELINE_FILTER] = true;
        itsBaselineFilter = filter;
    }
}

void VisSelection::setCorrelationFilter(const CorrelationFilter &filter)
{
    if(!filter.empty())
    {
        itsFlags[CORRELATION_FILTER] = true;
        itsCorrelationFilter = filter;
    }
}

const BaselineFilter &VisSelection::getBaselineFilter() const
{
    return itsBaselineFilter;
}

const CorrelationFilter &VisSelection::getCorrelationFilter() const
{
    return itsCorrelationFilter;
}

bool VisSelection::convertTime(const string &in, double &out) const
{
    //# TODO: Convert from default epoch to MS epoch (as it may differ from
    //# the default!)
    casa::Quantity time;

    if(in.empty() || !casa::MVTime::read(time, in))
        return false;

    out = time.getValue("s");
    return true;
}

} //# namespace BBS
} //# namespace LOFAR
