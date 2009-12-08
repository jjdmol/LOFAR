//# PiercePoint.h: Pierce point for a direction (az, el) on the sky.
//#
//# Copyright (C) 2007
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

#ifndef LOFAR_BBSKERNEL_EXPR_PIERCEPOINT_H
#define LOFAR_BBSKERNEL_EXPR_PIERCEPOINT_H

// \file
// Pierce point for a direction (az, el) on the sky.

#include <BBSKernel/Expr/BasicExpr.h>

#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PiercePoint: public BasicUnaryExpr<Vector<2>, Vector<4> >
{
public:
    typedef shared_ptr<PiercePoint>         Ptr;
    typedef shared_ptr<const PiercePoint>   ConstPtr;

    PiercePoint(const casa::MPosition &position,
        const Expr<Vector<2> >::ConstPtr &azel);

protected:
    virtual const Vector<4>::View evaluateImpl(const Grid &grid,
        const Vector<2>::View &azel) const;

private:
    // Station position (ITRF).
    casa::MPosition itsPosition;
    // (longitude, latittude) coordinates of station.
    double          itsLon, itsLat;
    // Height above earth-surface of station.
    double          itsHeight;
    // Earth radius at long lat of station.
    double          itsEarthRadius;
    // Ionosphere height in m.
    static const double theirIonosphereHeight;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
