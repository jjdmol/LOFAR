//# AntennaFieldAzEl.h: Compute azimuth and elevation in radians relevative to
//# the antenna field coordinate system (P, Q, R). The input direction as well
//# as the positive coordinate axes are assumed to be unit vectors expressed in
//# ITRF. Zero azimuth corresponds to the positive Q axis and positive azimuth
//# runs from the positive Q axis to the positive P axis. Elevation is the angle
//# the direction makes with the (P, Q) plane.
//#
//# Copyright (C) 2011
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

#ifndef LOFAR_BBSKERNEL_EXPR_ANTENNAFIELDAZEL_H
#define LOFAR_BBSKERNEL_EXPR_ANTENNAFIELDAZEL_H

// \file
// Compute azimuth and elevation in radians relevative to the antenna field
// coordinate system (P, Q, R). The input direction as well as the positive
// coordinate axes are assumed to be unit vectors expressed in ITRF. Zero
// azimuth corresponds to the positive Q axis and positive azimuth runs from the
// positive Q axis to the positive P axis. Elevation is the angle the direction
// makes with the (P, Q) plane.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Instrument.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class AntennaFieldAzEl: public BasicUnaryExpr<Vector<3>, Vector<2> >
{
public:
    typedef shared_ptr<AntennaFieldAzEl>        Ptr;
    typedef shared_ptr<const AntennaFieldAzEl>  ConstPtr;

    AntennaFieldAzEl(const Expr<Vector<3> >::ConstPtr &direction,
        const AntennaField::ConstPtr &field);

protected:
    virtual const Vector<2>::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &direction) const;

private:
    AntennaField::ConstPtr  itsField;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
