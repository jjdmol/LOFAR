//# AzEl.h: Azimuth and elevation for a direction (ra, dec) on the sky.
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

#ifndef LOFAR_BBSKERNEL_EXPR_AZEL_H
#define LOFAR_BBSKERNEL_EXPR_AZEL_H

// \file
// Azimuth and elevation for a direction (ra, dec) on the sky.

#include <BBSKernel/Expr/BasicExpr.h>

#include <measures/Measures/MPosition.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// Compute azimuth and elevation coordinates for a direction (ra, dec) (J2000)
// on the sky as seen from a specific location on earth.
class AzEl: public BasicUnaryExpr<Vector<2>, Vector<2> >
{
public:
    typedef shared_ptr<AzEl>        Ptr;
    typedef shared_ptr<const AzEl>  ConstPtr;

    AzEl(const casa::MPosition &position,
        const Expr<Vector<2> >::ConstPtr &direction);

protected:
    virtual const Vector<2>::View evaluateImpl(const Request &request,
        const Vector<2>::View &direction) const;

private:
    casa::MPosition itsPosition;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
