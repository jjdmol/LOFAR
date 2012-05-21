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
#include <Common/lofar_math.h>
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

    string          name;
    Position        position;
    vector<PointSource>  sources;

    size_t size() const { return sources.size(); }
    const PointSource &operator[](size_t i) const
    {
        return sources[i];
    }

    const_iterator begin() const
    {
        return sources.begin();
    }

    const_iterator end() const
    {
        return sources.end();
    }

    void syncPos()
    {
        Position position = sources.front().position();
        double cosDec = cos(position[1]);
        double x = cos(position[0]) * cosDec;
        double y = sin(position[0]) * cosDec;
        double z = sin(position[1]);

        for(unsigned int i = 1; i < sources.size(); ++i)
        {
            position = sources[i].position();
            cosDec = cos(position[1]);
            x += cos(position[0]) * cosDec;
            y += sin(position[0]) * cosDec;
            z += sin(position[1]);
        }

        x /= size();
        y /= size();
        z /= size();

        this->position[0] = atan2(y, x);
        this->position[1] = asin(z);
    }
};

// @}

} //# namespace DPPP
} //# namespace LOFAR

#endif
