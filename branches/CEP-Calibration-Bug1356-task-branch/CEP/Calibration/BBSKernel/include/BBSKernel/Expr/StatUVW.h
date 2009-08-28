//# StatUVW.h: UVW coordinates of a station in meters.
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef EXPR_STATUVW_H
#define EXPR_STATUVW_H

// \file
// UVW coordinates of a station in meters.

#include <BBSKernel/Expr/Expr.h>

#include <Common/lofar_map.h>
#include <Common/lofar_smartptr.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class StatUVW: public Expr<Vector<3> >
{
public:
    typedef shared_ptr<StatUVW>          Ptr;
    typedef shared_ptr<const StatUVW>    ConstPtr;

    StatUVW(const casa::MPosition &position, const casa::MPosition &array,
        const casa::MDirection &reference);

//    void calculate(const Request &request) const;

//    const Scalar getU(const Request &request) const
//    { if(request.id() != itsLastReqId) calculate(request); return itsU; }
//    const Scalar getV(const Request &request) const
//    { if(request.id() != itsLastReqId) calculate(request); return itsV; }
//    const Scalar getW(const Request &request) const
//    { if(request.id() != itsLastReqId) calculate(request); return itsW; }

protected:
    virtual unsigned int nArguments() const
    {
        return 0;
    }

    virtual ExprBase::ConstPtr argument(unsigned int) const
    {
        ASSERTSTR(false, "StatUVW has no arguments.");
    }

private:
    virtual const Vector<3> evaluateExpr(const Request &request, Cache &cache)
        const;

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

    casa::MPosition         itsPosition;
    casa::MPosition         itsArrayPosition;
    casa::MDirection        itsPhaseReference;

    mutable map<Time, Uvw>  itsUvwCache;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
