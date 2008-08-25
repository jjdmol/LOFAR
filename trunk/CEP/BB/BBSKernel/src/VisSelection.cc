//# VisSelection.cc: 
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
    itsFieldFlags.resize(N_FieldEnum, false);
}


bool VisSelection::empty() const
{
    return count(itsFieldFlags.begin(), itsFieldFlags.end(), true) == 0;
}


void VisSelection::setStartChannel(size_t start)
{
    if(!isSet(CHANNEL_START)
        && (!isSet(CHANNEL_END) || start <= itsChannelRange.second))
    {
        itsFieldFlags[CHANNEL_START] = true;
        itsChannelRange.first = start;
    }
    else if(start < itsChannelRange.first)
        itsChannelRange.first = start;
}


void VisSelection::setEndChannel(size_t end)
{
    if(!isSet(CHANNEL_END)
        && (!isSet(CHANNEL_START) || end >= itsChannelRange.first))
    {
        itsFieldFlags[CHANNEL_END] = true;
        itsChannelRange.second = end;
    }
    else if(end > itsChannelRange.second)
        itsChannelRange.second = end;
}


void VisSelection::setStartTime(double start)
{
    if(!isSet(TIME_START)
        && (!isSet(TIME_END) || start <= itsTimeRange.second))
    {
        itsFieldFlags[TIME_START] = true;
        itsTimeRange.first = start;
    }
    else if(start < itsTimeRange.first)
    {
        itsTimeRange.first = start;
    }
}


void VisSelection::setEndTime(double end)
{
        if(!isSet(TIME_END)
            && (!isSet(TIME_START) || end >= itsTimeRange.first))
        {
            itsFieldFlags[TIME_END] = true;
            itsTimeRange.second = end;
        }
        else if(end > itsTimeRange.second)
        {
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


void VisSelection::setPolarizations(vector<string> polarizations)
{
    set<string> selection(polarizations.begin(), polarizations.end());

    if(!isSet(POLARIZATIONS))
    {
        itsPolarizations = selection;
        itsFieldFlags[POLARIZATIONS] = true;
    }
    else
    {
        set<string>::const_iterator it = itsPolarizations.begin();
        while(it != itsPolarizations.end())
        {
            if(selection.count(*it))
                ++it;
            else
                itsPolarizations.erase(it++);
        }
    }
}


void VisSelection::setStations(vector<string> stations)
{
    set<string> selection(stations.begin(), stations.end());

    if(!isSet(STATIONS))
    {
        itsStations = selection;
        itsFieldFlags[STATIONS] = true;
    }
    else
    {
        set<string>::const_iterator it = itsStations.begin();
        while(it != itsStations.end())
        {
            if(selection.count(*it))
                ++it;
            else
                itsStations.erase(it++);
        }
    }
}


void VisSelection::setBaselineFilter(BaselineFilter filter)
{
    itsFieldFlags[BASELINE_FILTER] = true;
    itsBaselineFilter = filter;
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
