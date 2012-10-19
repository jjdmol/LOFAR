//# AntennaFieldThetaPhi.h: Compute topocentric (local) theta and phi spherical
//# coordinates (in radians) relative to the Cartesian antenna field coordinate
//# system (PQR), for a given target direction. The target direction is assumed
//# to be an ITRF unit vector in the direction of arrival. The positive
//# coordinate axes are assumed to be ITRF unit vectors. Zero phi corresponds to
//# the positive P axis, and positive phi runs from the positive P axis towards
//# the positive Q axis (roughly East over North). Theta or zenith angle is the
//# angle the target direction makes with the positive R axis.
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

#ifndef LOFAR_BBSKERNEL_EXPR_ANTENNAFIELDTHETAPHI_H
#define LOFAR_BBSKERNEL_EXPR_ANTENNAFIELDTHETAPHI_H

// \file
// Compute topocentric (local) theta and phi spherical coordinates (in radians)
// relative to the Cartesian antenna field coordinate system (PQR), for a given
// target direction. The target direction is assumed to be an ITRF unit vector
// in the direction of arrival. The positive coordinate axes are assumed to be
// ITRF unit vectors. Zero phi corresponds to the positive P axis, and positive
// phi runs from the positive P axis towards the positive Q axis (roughly East
// over North). Theta or zenith angle is the angle the target direction makes
// with the positive R axis.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Instrument.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class AntennaFieldThetaPhi: public BasicUnaryExpr<Vector<3>, Vector<2> >
{
public:
    typedef shared_ptr<AntennaFieldThetaPhi>        Ptr;
    typedef shared_ptr<const AntennaFieldThetaPhi>  ConstPtr;

    AntennaFieldThetaPhi(const Expr<Vector<3> >::ConstPtr &direction,
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
