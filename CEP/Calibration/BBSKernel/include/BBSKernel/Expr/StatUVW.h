//# StatUVW.h: UVW coordinates of a station in meters.
//#
//# Copyright (C) 2002
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

#ifndef EXPR_STATUVW_H
#define EXPR_STATUVW_H

// \file
// UVW coordinates of a station in meters.

#include <BBSKernel/Instrument.h>
#include <BBSKernel/Expr/Result.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/PhaseRef.h>

#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>
#include <Common/lofar_string.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class StatUVW
{
public:
    typedef shared_ptr<StatUVW>          Pointer;
    typedef shared_ptr<const StatUVW>    ConstPointer;

    StatUVW(const Station &station, const casa::MPosition &arrayRef,
        const PhaseRef::ConstPointer &phaseRef);
    ~StatUVW();

    void calculate(const Request &request) const;

    const Result &getU(const Request &request) const
    { if(request.getId() != itsLastReqId) calculate(request); return itsU; }
    const Result &getV(const Request &request) const
    { if(request.getId() != itsLastReqId) calculate(request); return itsV; }
    const Result &getW(const Request &request) const
    { if(request.getId() != itsLastReqId) calculate(request); return itsW; }

    const string &getName() const
    { return itsStation.name; }

private:
    struct Time
    {
        Time(double time)
            : time(time)
        {}

        bool operator<(const Time &other) const
        { return time < other.time - 0.000001; }

        double time;
    };

    struct Uvw
    {
        Uvw()
        {}

        Uvw(double u, double v, double w)
            : u(u), v(v), w(w)
        {}

        double u, v, w;
    };


    Station                  itsStation;
    casa::MPosition          itsArrayRef;
    PhaseRef::ConstPointer   itsPhaseRef;
    mutable map<Time, Uvw>   itsUvwCache;

    mutable Result           itsU;
    mutable Result           itsV;
    mutable Result           itsW;
    mutable RequestId        itsLastReqId;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
