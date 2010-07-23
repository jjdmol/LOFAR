//# VisDimensions.cc: Represents the dimensions of a block of visibility data
//# along for axes (frequency, time, baseline, correlation).
//#
//# Copyright (C) 2008
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

#include <BBSKernel/VisDimensions.h>
#include <Common/lofar_iomanip.h>

#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/MVTime.h>

namespace LOFAR
{
namespace BBS
{
using LOFAR::operator<<;

void VisDimensions::setGrid(const Grid &grid)
{
    itsGrid = grid;
}

void VisDimensions::setBaselines(const BaselineSeq &axis)
{
    itsBaselineAxis = axis;
}

void VisDimensions::setCorrelations(const CorrelationSeq &axis)
{
    itsCorrelationAxis = axis;
}

ostream &operator<<(ostream &out, const VisDimensions &obj)
{
    pair<double, double> freqRange = obj[FREQ]->range();
    pair<double, double> timeRange = obj[TIME]->range();

    out << "Frequency     : "
        << setprecision(3) << freqRange.first / 1e6 << " MHz"
        << " - "
        << setprecision(3) << freqRange.second / 1e6 << " MHz" << endl;
    out << "Bandwidth     : "
        << setprecision(3) << (freqRange.second - freqRange.first) / 1e6
        << " MHz (" << obj.nFreq() << " channel(s) of " << setprecision(3)
        << (freqRange.second - freqRange.first) / obj.nFreq() << " Hz)" << endl;
    out << "Time          : "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.first, "s"))
        << " - "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.second, "s"))
        << endl;
    out << "Duration      : "
        << setprecision(3) << (timeRange.second - timeRange.first) / 3600.0
        << " hour (" << obj.nTime() << " sample(s) of " << setprecision(3)
        << (timeRange.second - timeRange.first) / obj.nTime()
        << " s on average)" << endl;
    out << "Baseline count: " << obj.nBaselines() << endl;
    out << "Correlation(s): " << obj.correlations();

    return out;
}

} //# namespace BBS
} //# namespace LOFAR
