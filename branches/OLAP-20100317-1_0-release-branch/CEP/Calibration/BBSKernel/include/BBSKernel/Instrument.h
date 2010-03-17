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

class AntennaSelection
{
public:
    AntennaSelection();
    AntennaSelection(unsigned int size);

    const string &name() const;
    unsigned int size() const;
    const double &operator()(unsigned int i0, unsigned int i1) const;
    double &operator()(unsigned int i0, unsigned int i1);

private:
    friend istream &operator>>(istream &in, AntennaSelection &obj);

    string                  itsName;
    casa::Matrix<double>    itsPositions;
};

istream &operator>>(istream &in, AntennaSelection &obj);

class TileLayout
{
public:
    TileLayout();
    TileLayout(unsigned int size);

    unsigned int size() const;
    const double &operator()(unsigned int i0, unsigned int i1) const;
    double &operator()(unsigned int i0, unsigned int i1);

private:
    casa::Matrix<double>    itsPositions;
};

class Station
{
public:
    Station(const string &name, const casa::MPosition &position);

    const string &name() const;
    const casa::MPosition &position() const;
    const AntennaSelection &selection(const string &name) const;
    const TileLayout &tile(unsigned int i) const;

    void readAntennaSelection(const casa::Path &file);
    void readTileLayout(const casa::Path &file);

private:
    string                          itsName;
    casa::MPosition                 itsPosition;
    map<string, AntennaSelection>   itsAntennaSelection;
    vector<TileLayout>              itsTileLayout;
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

    void readLOFARAntennaConfig(const casa::Path &path);

private:
    string                      itsName;
    casa::MPosition             itsPosition;
    map<string, unsigned int>   itsIndex;
    vector<Station>             itsStations;
};

// @}

// -------------------------------------------------------------------------- //
// - AntennaSelection implementation                                        - //
// -------------------------------------------------------------------------- //

inline const double &AntennaSelection::operator()(unsigned int i0,
    unsigned int i1) const
{
    //# Swap axes to hide the Fortran index convention used by casa::Array.
    return itsPositions(i1, i0);
}

inline double &AntennaSelection::operator()(unsigned int i0, unsigned int i1)
{
    //# Swap axes to hide the Fortran index convention used by casa::Array.
    return itsPositions(i1, i0);
}

// -------------------------------------------------------------------------- //
// - TileLayout implementation                                              - //
// -------------------------------------------------------------------------- //

inline const double &TileLayout::operator()(unsigned int i0, unsigned int i1)
    const
{
    //# Swap axes to hide the Fortran index convention used by casa::Array.
    return itsPositions(i1, i0);
}

inline double &TileLayout::operator()(unsigned int i0, unsigned int i1)
{
    //# Swap axes to hide the Fortran index convention used by casa::Array.
    return itsPositions(i1, i0);
}

} // namespace BBS
} // namespace LOFAR

#endif
