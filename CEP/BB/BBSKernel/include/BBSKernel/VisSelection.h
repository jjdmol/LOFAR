//# VisSelection.h: 
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
//        FREQ_START = 0,
//        FREQ_END,
        CHANNEL_START = 0,
        CHANNEL_END,
//        CHANNEL_STEP,
        TIME_START,
        TIME_END,
        STATIONS,
        CORRELATIONS,
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

    void setChannelRange(size_t start, size_t end);
    void setTimeRange(string start, string end);
    void setTimeRange(double start, double end);
    void setCorrelations(vector<string> correlations);
    void setStations(vector<string> stations);
    void setBaselineFilter(BaselineFilter filter);

    pair<size_t, size_t> getChannelRange() const
    { return itsChannelRange; }

//    size_t getChannelStep() const
//    { return itsChannelStep; }

    pair<double, double> getTimeRange() const
    { return itsTimeRange; }

    set<string> getCorrelations() const
    { return itsCorrelations; }

    set<string> getStations() const
    { return itsStations; }

    BaselineFilter getBaselineFilter() const
    { return itsBaselineFilter; }

//    void selectFreqRange(const pair<uint, uint> &range);
//    void selectBaselines(const vector<string> &stations);
//    void selectBaselines(
//        const pair<vector<string>, vector<string> > &baselines);

private:
    bool convertTime(const string &in, double &out) const;
    vector<bool>            itsFieldFlags;
    pair<size_t, size_t>    itsChannelRange;
//    size_t                  itsChannelStep;
//    pair<double, double>    itsFreqRange;
    pair<double, double>    itsTimeRange;
//    set<pair<uint, uint> >  itsBaselines;
    set<string>             itsCorrelations;
    set<string>             itsStations;
    BaselineFilter          itsBaselineFilter;
};

} //# namespace BBS
} //# namespace LOFAR

#endif
