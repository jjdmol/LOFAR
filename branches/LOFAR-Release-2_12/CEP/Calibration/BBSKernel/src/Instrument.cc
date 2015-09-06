//# Instrument.cc: Description of the telescope.
//#
//# Copyright (C) 2009
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
#include <BBSKernel/Instrument.h>
#include <BBSKernel/Exceptions.h>

namespace LOFAR
{
namespace BBS
{

Station::Station(const string &name, const casa::MPosition &position)
    :   itsName(name),
        itsPosition(position)
{
}

Station::~Station()
{
}

const string &Station::name() const
{
    return itsName;
}

const casa::MPosition &Station::position() const
{
    return itsPosition;
}

Instrument::Instrument(const string &name, const casa::MPosition &position)
    :   itsName(name),
        itsPosition(position)
{
}

const string &Instrument::name() const
{
    return itsName;
}

const casa::MPosition &Instrument::position() const
{
    return itsPosition;
}

unsigned int Instrument::nStations() const
{
    return itsStations.size();
}

Station::ConstPtr Instrument::station(unsigned int i) const
{
    return itsStations[i];
}

Station::ConstPtr Instrument::station(const string &name) const
{
    map<string, unsigned int>::const_iterator it = itsIndex.find(name);
    if(it == itsIndex.end())
    {
        THROW(BBSKernelException, "Unknown station: " << name);
    }

    return itsStations[it->second];
}

void Instrument::append(const Station::Ptr &station)
{
    itsStations.push_back(station);
}

} //# namespace BBS
} //# namespace LOFAR
