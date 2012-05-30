//# Patch.h: A set of sources for which direction dependent effects are assumed
//# to be equal.
//#
//# Copyright (C) 2012
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

#ifndef DPPP_PATCH_H
#define DPPP_PATCH_H

// \file
// A set of sources for which direction dependent effects are assumed to be
// equal.

#include <DPPP/PointSource.h>
#include <DPPP/Position.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
namespace DPPP
{

// \addtogroup NDPPP
// @{

struct Patch
{
    typedef vector<PointSource>::const_iterator const_iterator;

    Patch();

    template <typename T>
    Patch(const string &name, T first, T last);

    const string &name() const;
    const Position &position() const;

    size_t nComponents() const;
    const PointSource &operator[](size_t i) const;

    const_iterator begin() const;
    const_iterator end() const;

private:
    void recomputePosition();

    string              itsName;
    Position            itsPosition;
    vector<PointSource> itsComponents;
};

// @}

// -------------------------------------------------------------------------- //
// - Implementation: Patch                                                  - //
// -------------------------------------------------------------------------- //

template <typename T>
Patch::Patch(const string &name, T first, T last)
    :   itsName(name),
        itsComponents(first, last)
{
    recomputePosition();
}

inline const string &Patch::name() const
{
    return itsName;
}

inline const Position &Patch::position() const
{
    return itsPosition;
}

inline size_t Patch::nComponents() const
{
    return itsComponents.size();
}

inline const PointSource &Patch::operator[](size_t i) const
{
    return itsComponents[i];
}

inline Patch::const_iterator Patch::begin() const
{
    return itsComponents.begin();
}

inline Patch::const_iterator Patch::end() const
{
    return itsComponents.end();
}

} //# namespace DPPP
} //# namespace LOFAR

#endif
