//# StationLOFAR.h: Description of a LOFAR station.
//#
//# Copyright (C) 2013
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

#ifndef LOFAR_BBSKERNEL_STATIONLOFAR_H
#define LOFAR_BBSKERNEL_STATIONLOFAR_H

// \file
// Description of a LOFAR station.

#include <BBSKernel/Instrument.h>
#include <StationResponse/Station.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup BBSKernel
// @{

class StationLOFAR: public Station
{
public:
    typedef shared_ptr<StationLOFAR>        Ptr;
    typedef shared_ptr<const StationLOFAR>  ConstPtr;

    StationLOFAR(const string &name, const casa::MPosition &position,
        const StationResponse::Station::ConstPtr &station);

    StationResponse::Station::ConstPtr station() const;

private:
    StationResponse::Station::ConstPtr  itsStation;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
