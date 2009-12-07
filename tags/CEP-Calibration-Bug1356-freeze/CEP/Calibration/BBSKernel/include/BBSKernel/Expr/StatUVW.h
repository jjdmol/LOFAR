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

#ifndef LOFAR_BBSKERNEL_EXPR_STATUVW_H
#define LOFAR_BBSKERNEL_EXPR_STATUVW_H

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

protected:
    virtual unsigned int nArguments() const;
    virtual ExprBase::ConstPtr argument(unsigned int) const;
    virtual const Vector<3> evaluateExpr(const Request &request, Cache &cache,
        unsigned int grid) const;

private:
    struct Timestamp
    {
        Timestamp(double time);
        bool operator<(const Timestamp &other) const;

        double time;
    };

    struct UVW
    {
        UVW();
        UVW(double u, double v, double w);

        double u, v, w;
    };

    casa::MPosition             itsPosition;
    casa::MPosition             itsArrayPosition;
    casa::MDirection            itsPhaseReference;

    mutable map<Timestamp, UVW> itsUVWCache;
};

// @}


// -------------------------------------------------------------------------- //
// - Implementation: StatUVW                                                - //
// -------------------------------------------------------------------------- //

inline unsigned int StatUVW::nArguments() const
{
    return 0;
}

inline ExprBase::ConstPtr StatUVW::argument(unsigned int) const
{
    ASSERTSTR(false, "StatUVW has no arguments.");
}

// -------------------------------------------------------------------------- //
// - Implementation: StatUVW::Timestamp                                     - //
// -------------------------------------------------------------------------- //

inline StatUVW::Timestamp::Timestamp(double time)
    :   time(time)
{
}

inline bool StatUVW::Timestamp::operator<(const Timestamp &other) const
{
    return time < other.time - 0.000001;
}

// -------------------------------------------------------------------------- //
// - Implementation: StatUVW::UVW                                           - //
// -------------------------------------------------------------------------- //

inline StatUVW::UVW::UVW()
{
}

inline StatUVW::UVW::UVW(double u, double v, double w)
    :   u(u),
        v(v),
        w(w)
{
}

} // namespace BBS
} // namespace LOFAR

#endif
