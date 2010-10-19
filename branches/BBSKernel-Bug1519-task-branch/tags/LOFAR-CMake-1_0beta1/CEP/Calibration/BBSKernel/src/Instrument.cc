//# Instrument.cc:
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

namespace LOFAR
{
namespace BBS
{

const string &AntennaConfig::name() const
{
    return itsName;
}

size_t AntennaConfig::size() const
{
    return itsPositions.shape()[1];
}

istream &operator>>(istream &in, AntennaConfig &obj)
{
    string line;

    // Parse configuration name.
//    in >> ws;
    getline(in, line);
    size_t start = line.find_first_not_of(" \t\n");
    size_t end = line.find_first_of(" \t\n", start);

    if(start == string::npos)
    {
        in.setstate(istream::failbit);
        return in;
    }

    obj.itsName = line.substr(start, end);

    // Skip line containing phase center.
//    in >> ws;
    getline(in, line);

    // Parse array shape.
    char sep;
    unsigned int shape[3];
    in >> shape[0] >> sep >> shape[1] >> sep >> shape[2] >> sep;

    if(!in.good() || sep != '[' || shape[0] == 0 || shape[1] != 2
        || shape[2] != 3)
    {
        in.setstate(istream::failbit);
        return in;
    }

    // Parse array.
    obj.itsPositions.resize(3, shape[0]);

    double skip;
    for(unsigned int i = 0; in.good() && i < shape[0]; ++i)
    {
        in >> obj.itsPositions(0, i) >> obj.itsPositions(1, i)
            >> obj.itsPositions(2, i) >> skip >> skip >> skip;
    }

    in >> sep;

    if(!in.good() || sep != ']')
    {
        in.setstate(istream::failbit);
    }

//    LOG_DEBUG_STR("name: " << obj.itsName << " positions: "
//        << obj.itsPositions);
//    LOG_DEBUG_STR("STATION 0: " << obj(0, 0) << " " << obj(0, 1) << " "
//        << obj(0, 2));
    return in;
}

Station::Station(const string &name, const casa::MPosition &position)
    :   itsName(name),
        itsPosition(position)
{
}

Station::Station(const string &name, const casa::MPosition &position,
    const casa::Path &config)
    :   itsName(name),
        itsPosition(position)
{
    readAntennaConfigurations(config);
}

const string &Station::name() const
{
    return itsName;
}

const casa::MPosition &Station::position() const
{
    return itsPosition;
}

const AntennaConfig &Station::config(const string &name) const
{
    map<string, AntennaConfig>::const_iterator it = itsConfig.find(name);
    if(it == itsConfig.end())
    {
        THROW(BBSKernelException, "Unknown antenna configuration for station "
            << this->name() << ": " << name);
    }

    return it->second;
}

void Station::readAntennaConfigurations(const casa::Path &file)
{
    casa::String path(file.expandedName());

    ifstream ifs;
    ifs.exceptions(ifstream::failbit | ifstream::badbit);

    map<string, AntennaConfig> result;
    try
    {
        ifs.open(path.c_str());

        while(ifs.good())
        {
            ifs >> ws;

            if(ifs.eof())
            {
                break;
            }

            if(ifs.peek() == '#')
            {
                string skip;
                getline(ifs, skip);
                continue;
            }

            AntennaConfig config;
            ifs >> config;
            result[config.name()] = config;
        }
    }
    catch(ifstream::failure &e)
    {
        THROW(BBSKernelException, "Error opening/reading antenna configuration"
          " file: " << path);
    }

    itsConfig = result;
}

Instrument::Instrument()
{
}

Instrument::Instrument(const string &name, const casa::MPosition position,
    const vector<Station> &stations)
    :   itsName(name),
        itsPosition(position),
        itsStations(stations)
{
    for(unsigned int i = 0; i < itsStations.size(); ++i)
    {
        itsIndex[itsStations[i].name()] = i;
    }
}

const string &Instrument::name() const
{
    return itsName;
}

const casa::MPosition &Instrument::position() const
{
    return itsPosition;
}

unsigned int Instrument::size() const
{
    return itsStations.size();
}

const Station &Instrument::operator[](unsigned int i) const
{
    DBGASSERT(i < itsStations.size());
    return itsStations[i];
}

const Station &Instrument::operator[](const string &name) const
{
    map<string, unsigned int>::const_iterator it = itsIndex.find(name);
    if(it == itsIndex.end())
    {
        THROW(BBSKernelException, "Unknown station: " << name);
    }

    return itsStations[it->second];
}

void Instrument::readAntennaConfigurations(const casa::Path &path)
{
    for(unsigned int i = 0; i < itsStations.size(); ++i)
    {
        casa::Path file(path);
        file.append(itsStations[i].name() + "-AntennaArrays.conf");
        itsStations[i].readAntennaConfigurations(file);
    }
}

} //# namespace BBS
} //# namespace LOFAR
