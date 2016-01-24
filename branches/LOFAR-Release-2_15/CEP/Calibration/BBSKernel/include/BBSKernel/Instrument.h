//# Instrument.h: Description of the telescope.
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

// \file
// Description of the telescope.

#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class Station
{
public:
    typedef shared_ptr<Station>       Ptr;
    typedef shared_ptr<const Station> ConstPtr;

    Station(const string &name, const casa::MPosition &position);
    virtual ~Station();

    const string &name() const;
    const casa::MPosition &position() const;

private:
    string                      itsName;
    casa::MPosition             itsPosition;
};

class Instrument
{
public:
    typedef shared_ptr<Instrument>        Ptr;
    typedef shared_ptr<const Instrument>  ConstPtr;

    Instrument(const string &name, const casa::MPosition &position);

    template <typename T>
    Instrument(const string &name, const casa::MPosition &position, T first,
        T last);

    const string &name() const;
    const casa::MPosition &position() const;

    unsigned int nStations() const;
    Station::ConstPtr station(unsigned int i) const;
    Station::ConstPtr station(const string &name) const;

    void append(const Station::Ptr &station);

private:
    string                      itsName;
    casa::MPosition             itsPosition;
    map<string, unsigned int>   itsIndex;
    vector<Station::Ptr>        itsStations;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: Instrument                                             - //
// -------------------------------------------------------------------------- //

template <typename T>
Instrument::Instrument(const string &name, const casa::MPosition &position,
    T first, T last)
    :   itsName(name),
        itsPosition(position),
        itsStations(first, last)
{
}

} // namespace BBS
} // namespace LOFAR

#endif
