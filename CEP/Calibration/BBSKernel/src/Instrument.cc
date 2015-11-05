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
#include <Common/lofar_fstream.h>
#include <Common/lofar_string.h>
#include <BBSKernel/Instrument.h>
#include <BBSKernel/Exceptions.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCPosition.h>

namespace LOFAR
{
namespace BBS
{

AntennaField::AntennaField(const string &name, const Vector3 &position,
    const Vector3 &p, const Vector3 &q, const Vector3 &r)
    :   itsName(name),
        itsPosition(position),
        itsActiveElementCount(0)
{
    itsAxes[P] = p;
    itsAxes[Q] = q;
    itsAxes[R] = r;
}

const string &AntennaField::name() const
{
    return itsName;
}

const Vector3 &AntennaField::position() const
{
    return itsPosition;
}

const Vector3 &AntennaField::axis(Axis axis) const
{
    return itsAxes[axis];
}

bool AntennaField::isHBA() const
{
    return itsName != "LBA";
}

void AntennaField::appendTileElement(const Vector3 &offset)
{
    itsTileElements.push_back(offset);
}

void AntennaField::appendElement(const Element &element)
{
    itsElements.push_back(element);

    if(!element.flag[0] || !element.flag[1])
    {
        ++itsActiveElementCount;
    }
}

Station::Station(const string &name, const casa::MPosition &position)
    :   itsName(name),
        itsPosition(position)
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

bool Station::isPhasedArray() const
{
    return !itsFields.empty();
}

size_t Station::nElement() const
{
    size_t count = 0;
    for(unsigned int i = 0; i < nField(); ++i)
    {
        count += field(i)->nElement();
    }
    return count;
}

size_t Station::nActiveElement() const
{
    size_t count = 0;
    for(unsigned int i = 0; i < nField(); ++i)
    {
        count += field(i)->nActiveElement();
    }
    return count;
}

unsigned int Station::nField() const
{
    return itsFields.size();
}

AntennaField::ConstPtr Station::field(unsigned int i) const
{
    return itsFields[i];
}

void Station::append(const AntennaField::Ptr &field)
{
    itsFields.push_back(field);
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
