//# BaselineMask.cc: Baseline selection in the form of a boolean baseline mask.
//# Can decide if a given baseline is selected in O(1) time.
//#
//# Copyright (C) 2010
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
#include <BBSKernel/BaselineMask.h>

namespace LOFAR
{
namespace BBS
{

BaselineMask::BaselineMask(size_t nStations, bool initial)
    :   itsUpdateStationMask(false),
        itsStationMask(nStations, initial),
        itsMask(nStations * nStations, initial)
{
}

void BaselineMask::mask(size_t i)
{
    itsStationMask[i] = true;
    for(size_t j = 0; j < nStations(); ++j)
    {
        itsMask[i * nStations() + j] = true;
        itsMask[j * nStations() + i] = true;
    }
}

void BaselineMask::mask(size_t i, size_t j)
{
    itsStationMask[i] = true;
    itsStationMask[j] = true;
    itsMask[i * nStations() + j] = true;
}

void BaselineMask::mask(const baseline_t &bl)
{
    mask(bl.first, bl.second);
}

void BaselineMask::unmask(size_t i)
{
    itsStationMask[i] = false;
    for(size_t j = 0; j < nStations(); ++j)
    {
        itsMask[i * nStations() + j] = false;
        itsMask[j * nStations() + i] = false;
    }
}

void BaselineMask::unmask(size_t i, size_t j)
{
    itsMask[i * nStations() + j] = false;
    itsUpdateStationMask = true;
}

void BaselineMask::unmask(const baseline_t &bl)
{
    unmask(bl.first, bl.second);
}

void BaselineMask::updateStationMask() const
{
    fill(itsStationMask.begin(), itsStationMask.end(), false);

    for(size_t i = 0; i < nStations(); ++i)
    {
        if(!itsStationMask[i])
        {
            for(size_t j = 0; j < nStations(); ++j)
            {
                if(this->operator()(i, j) || this->operator()(j, i))
                {
                    itsStationMask[i] = true;
                    itsStationMask[j] = true;
                    break;
                }
            }
        }
    }

    itsUpdateStationMask = false;
}

} //# namespace BBS
} //# namespace LOFAR
