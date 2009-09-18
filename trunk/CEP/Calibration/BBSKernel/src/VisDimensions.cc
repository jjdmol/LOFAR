//# VisDimensions.cc: 
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
#include <BBSKernel/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
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


void VisDimensions::setBaselines(const vector<baseline_t> &baselines)
{
    itsBaselineIndex.clear();
    itsBaselines = baselines;
        
    for(size_t i = 0; i < itsBaselines.size(); ++i)
    {
        itsBaselineIndex[itsBaselines[i]] = i;
    }
}


void VisDimensions::setPolarizations(const vector<string> &polarizations)
{
    itsPolarizationIndex.clear();
    itsPolarizations = polarizations;
        
    for(size_t i = 0; i < itsPolarizations.size(); ++i)
    {
        itsPolarizationIndex[itsPolarizations[i]] = i;
    }
}


bool VisDimensions::hasBaseline(baseline_t baseline) const
{
    return (itsBaselineIndex.find(baseline) != itsBaselineIndex.end());
}


size_t VisDimensions::getBaselineIndex(baseline_t baseline) const
{
    map<baseline_t, size_t>::const_iterator it =
        itsBaselineIndex.find(baseline);

    if(it != itsBaselineIndex.end())
    {
        return it->second;
    }
    THROW(BBSKernelException, "Request for index of unknown baseline "
        << baseline.first << " - " << baseline.second);
}


bool VisDimensions::hasPolarization(const string &polarization) const
{
    return (itsPolarizationIndex.find(polarization) !=
        itsPolarizationIndex.end());
}


size_t VisDimensions::getPolarizationIndex(const string &polarization) const
{
    map<string, size_t>::const_iterator it =
        itsPolarizationIndex.find(polarization);
        
    if(it != itsPolarizationIndex.end())
    {
        return it->second;
    }
    THROW(BBSKernelException, "Request for index of unknown polarization "
        << polarization);
}


ostream &operator<<(ostream &out, const VisDimensions &obj)
{
    pair<double, double> freqRange = obj.getFreqRange();
    pair<double, double> timeRange = obj.getTimeRange();

    out << "Frequency      : "
        << setprecision(3) << freqRange.first / 1e6 << " MHz"
        << " - "
        << setprecision(3) << freqRange.second / 1e6 << " MHz" << endl;
    out << "Bandwidth      : "
        << setprecision(3) << (freqRange.second - freqRange.first) / 1e3
        << " kHz (" << obj.getChannelCount() << " channel(s) of "
        << setprecision(3)
        << (freqRange.second - freqRange.first) / obj.getChannelCount()
        << " Hz)" << endl;
    out << "Time           : "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.first, "s"))
        << " - "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeRange.second, "s"))
        << endl;
    out << "Duration       : "
        << setprecision(3) << (timeRange.second - timeRange.first) / 3600.0
        << " hour (" << obj.getTimeslotCount() << " sample(s) of "
        << setprecision(3)
        << (timeRange.second - timeRange.first) / obj.getTimeslotCount()
        << " s on average)" << endl;
    out << "Baseline count : " << obj.getBaselineCount() << endl;
    out << "Polarization(s): " << obj.getPolarizations();
    
    return out;
}    

} //# namespace BBS
} //# namespace LOFAR
