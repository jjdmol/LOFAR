//# StationUVW.h: Station UVW coordinates.
//#
//# Copyright (C) 2009
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

#ifndef LOFAR_BBSKERNEL_EXPR_STATIONUVW_H
#define LOFAR_BBSKERNEL_EXPR_STATIONUVW_H

// \file
// Station UVW coordinates.

#include <BBSKernel/Expr/Expr.h>

#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class StationUVW: public Expr<Vector<3> >
{
public:
    typedef shared_ptr<StationUVW>          Ptr;
    typedef shared_ptr<const StationUVW>    ConstPtr;

    StationUVW(const casa::MPosition &position, const casa::MPosition &array,
        const casa::MDirection &reference);

protected:
    virtual unsigned int nArguments() const
    {
        return 0;
    }

    virtual ExprBase::ConstPtr argument(unsigned int) const
    {
        ASSERTSTR(false, "StationUVW has no arguments.");
    }

    virtual const Vector<3> evaluateExpr(const Request &request, Cache &cache)
        const;

private:
    casa::MPosition     itsPosition;
    casa::MPosition     itsArrayPosition;
    casa::MDirection    itsPhaseReference;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
