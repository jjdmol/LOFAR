//# VisSelection.h: Selection of visibility data that can exist independent of
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

#ifndef LOFAR_BBSKERNEL_VISSELECTION_H
#define LOFAR_BBSKERNEL_VISSELECTION_H

#include <Common/lofar_string.h>
#include <BBSKernel/BaselineFilter.h>
#include <BBSKernel/CorrelationMask.h>
#include <utility>

namespace LOFAR
{
namespace BBS
{
using std::pair;

// \addtogroup BBSKernel
// @{

class VisSelection
{
public:
    enum Field
    {
        CHANNEL_START,
        CHANNEL_END,
        TIME_START,
        TIME_END,
        BASELINE_FILTER,
        CORRELATION_MASK,
        N_Field
    };

    VisSelection();

    void clear(Field field);
    bool isSet(Field field) const;
    bool empty() const;

    void setStartChannel(size_t start);
    void setEndChannel(size_t end);
    void setChannelRange(size_t start, size_t end);

    void setStartTime(double start);
    void setEndTime(double end);
    void setTimeRange(double start, double end);

    void setStartTime(const string &start);
    void setEndTime(const string &end);
    void setTimeRange(const string &start, const string &end);

    void setBaselineFilter(const BaselineFilter &filter);
    void setCorrelationMask(const CorrelationMask &mask);

    pair<size_t, size_t> getChannelRange() const;
    pair<double, double> getTimeRange() const;
    const BaselineFilter &getBaselineFilter() const;
    const CorrelationMask &getCorrelationMask() const;

private:
    bool convertTime(const string &in, double &out) const;

    bool                    itsFlags[N_Field];
    pair<size_t, size_t>    itsChannelRange;
    pair<double, double>    itsTimeRange;
    BaselineFilter          itsBaselineFilter;
    CorrelationMask         itsCorrelationMask;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
