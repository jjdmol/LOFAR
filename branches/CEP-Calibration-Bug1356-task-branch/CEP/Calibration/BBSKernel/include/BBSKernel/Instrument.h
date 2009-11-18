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

#include <Common/lofar_string.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

struct Station
{
    string              name;
    casa::MPosition     position;
};

struct Instrument
{
    string              name;
    casa::MPosition     position;
    vector<Station>     stations;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
