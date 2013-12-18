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

#include <BBSKernel/Types.h>

#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>
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

class AntennaField
{
public:
    typedef shared_ptr<AntennaField>        Ptr;
    typedef shared_ptr<const AntennaField>  ConstPtr;

    enum Axis
    {
        P,
        Q,
        R,
        N_Axis
    };

    struct Element
    {
        Vector3 offset;
        bool    flag[2];
    };

    AntennaField(const string &name, const Vector3 &position, const Vector3 &p,
        const Vector3 &q, const Vector3 &r);

    const string &name() const;
    const Vector3 &position() const;
    const Vector3 &axis(Axis axis) const;

    bool isHBA() const;

    void appendTileElement(const Vector3 &offset);
    inline size_t nTileElement() const;
    inline const Vector3 &tileElement(size_t i) const;

    void appendElement(const Element &element);
    inline size_t nElement() const;
    inline size_t nActiveElement() const;
    inline const Element &element(size_t i) const;

private:
    string                  itsName;
    Vector3                 itsPosition;
    size_t                  itsActiveElementCount;
    Vector3                 itsAxes[N_Axis];
    vector<Vector3>         itsTileElements;
    vector<Element>         itsElements;
};

class Station
{
public:
    typedef shared_ptr<Station>       Ptr;
    typedef shared_ptr<const Station> ConstPtr;

    Station(const string &name, const casa::MPosition &position);

    template <typename T>
    Station(const string &name, const casa::MPosition &position, T first,
        T last);

    const string &name() const;
    const casa::MPosition &position() const;

    bool isPhasedArray() const;
    size_t nElement() const;
    size_t nActiveElement() const;

    unsigned int nField() const;
    AntennaField::ConstPtr field(unsigned int i) const;

    void append(const AntennaField::Ptr &field);

private:
    string                      itsName;
    casa::MPosition             itsPosition;
    vector<AntennaField::Ptr>   itsFields;
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
// - Implementation: AntennaField                                           - //
// -------------------------------------------------------------------------- //

inline size_t AntennaField::nTileElement() const
{
    return itsTileElements.size();
}

const Vector3 &AntennaField::tileElement(size_t i) const
{
    return itsTileElements[i];
}

inline size_t AntennaField::nElement() const
{
    return itsElements.size();
}

inline size_t AntennaField::nActiveElement() const
{
    return itsActiveElementCount;
}

inline const AntennaField::Element &AntennaField::element(size_t i) const
{
    return itsElements[i];
}

// -------------------------------------------------------------------------- //
// - Implementation: Station                                                - //
// -------------------------------------------------------------------------- //

template <typename T>
Station::Station(const string &name, const casa::MPosition &position, T first,
    T last)
    :   itsName(name),
        itsPosition(position),
        itsFields(first, last)
{
}

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
