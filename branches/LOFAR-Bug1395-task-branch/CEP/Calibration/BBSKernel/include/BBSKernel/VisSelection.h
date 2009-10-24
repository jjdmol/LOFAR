//# VisSelection.h: 
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

#ifndef LOFAR_BBS_BBSKERNEL_VISSELECTION_H
#define LOFAR_BBS_BBSKERNEL_VISSELECTION_H

#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_set.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

#include <utility>

namespace LOFAR
{
namespace BBS
{
using std::pair;

class VisSelection
{
public:
    enum FieldEnum
    {
        CHANNEL_START = 0,
        CHANNEL_END,
        TIME_START,
        TIME_END,
        STATIONS,
        POLARIZATIONS,
        BASELINE_FILTER,
        N_FieldEnum
    };

    enum BaselineFilter
    {
        AUTO = 0,
        CROSS,
        N_BaselineFilter
    };

    VisSelection();

    void clear(FieldEnum field)
    { itsFieldFlags[field] = false; }

    bool isSet(FieldEnum field) const
    { return itsFieldFlags[field]; }

    bool empty() const;
    
    void setStartChannel(size_t start);
    void setEndChannel(size_t end);
    void setChannelRange(size_t start, size_t end)
    {
        setStartChannel(start);
        setEndChannel(end);
    }

    void setStartTime(double start);
    void setEndTime(double end);
    void setTimeRange(double start, double end)
    {
        setStartTime(start);
        setEndTime(end);
    }

    void setStartTime(const string &start);
    void setEndTime(const string &end);
    void setTimeRange(const string &start, const string &end)
    {
        setStartTime(start);
        setEndTime(end);
    }

    void setPolarizations(vector<string> polarizations);
    void setStations(vector<string> stations);
    void setBaselineFilter(BaselineFilter filter);

    pair<size_t, size_t> getChannelRange() const
    { return itsChannelRange; }

    pair<double, double> getTimeRange() const
    { return itsTimeRange; }

    set<string> getPolarizations() const
    { return itsPolarizations; }

    set<string> getStations() const
    { return itsStations; }

    BaselineFilter getBaselineFilter() const
    { return itsBaselineFilter; }

private:
    bool convertTime(const string &in, double &out) const;
    
    vector<bool>            itsFieldFlags;
    pair<size_t, size_t>    itsChannelRange;
    pair<double, double>    itsTimeRange;
    set<string>             itsPolarizations;
    set<string>             itsStations;
    BaselineFilter          itsBaselineFilter;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
