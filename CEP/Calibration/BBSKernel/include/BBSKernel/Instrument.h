//# Instrument.h:
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

#ifndef LOFAR_BBSKERNEL_INSTRUMENT_H
#define LOFAR_BBSKERNEL_INSTRUMENT_H

#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

#include <casa/Arrays.h>
#include <casa/OS/Path.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class AntennaConfig
{
public:
    const string &name() const;
    size_t size() const;
    double operator()(unsigned int i, unsigned int j) const;

private:
    friend istream &operator>>(istream &in, AntennaConfig &obj);

    string                  itsName;
    casa::Matrix<double>    itsPositions;
};

istream &operator>>(istream &in, AntennaConfig &obj);

class Station
{
public:
//    Station();
    Station(const string &name, const casa::MPosition &position);
    Station(const string &name, const casa::MPosition &position,
        const casa::Path &config);

    const string &name() const;
    const casa::MPosition &position() const;
    const AntennaConfig &config(const string &name) const;

    void readAntennaConfigurations(const casa::Path &file);

private:
    string                      itsName;
    casa::MPosition             itsPosition;
    map<string, AntennaConfig>  itsConfig;
};

class Instrument
{
public:
    Instrument();
    Instrument(const string &name, const casa::MPosition position,
        const vector<Station> &stations);

    const string &name() const;
    const casa::MPosition &position() const;

    unsigned int size() const;
    const Station &operator[](unsigned int i) const;
    const Station &operator[](const string &name) const;

    void readAntennaConfigurations(const casa::Path &path);

private:
    string                      itsName;
    casa::MPosition             itsPosition;
    map<string, unsigned int>   itsIndex;
    vector<Station>             itsStations;
};

// @}

// -------------------------------------------------------------------------- //
// - AntennaConfig implementation                                           - //
// -------------------------------------------------------------------------- //

inline double AntennaConfig::operator()(unsigned int i, unsigned int j)
    const
{
    //# Swap axes to hide the Fortran index convention used by casa::Array.
    return itsPositions(j, i);
}

} // namespace BBS
} // namespace LOFAR

#endif
