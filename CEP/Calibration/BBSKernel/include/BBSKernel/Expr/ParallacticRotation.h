//# ParallacticRotation.h: Jones matrix that relates the (X,Y)-frame used to
//# express polarization on the sky (according to the IAU definition) to the
//# topocentric (theta,phi)-frame. Both are Cartesian frames defined on the
//# tangent plane, with +X towards the North, +Y towards the East, +theta away
//# from the (pseudo) zenith, and +phi East over North around the (pseudo)
//# zenith.
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

#ifndef LOFAR_BBSKERNEL_EXPR_PARALLACTICROTATION_H
#define LOFAR_BBSKERNEL_EXPR_PARALLACTICROTATION_H

// \file
// Jones matrix that relates the (X,Y)-frame used to express polarization on the
// sky (according to the IAU definition) to the topocentric (theta,phi)-frame.
// Both are Cartesian frames defined on the tangent plane, with +X towards the
// North, +Y towards the East, +theta away from the (pseudo) zenith, and +phi
// East over North around the (pseudo) zenith.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Instrument.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ParallacticRotation: public BasicUnaryExpr<Vector<3>, JonesMatrix>
{
public:
    typedef shared_ptr<ParallacticRotation>         Ptr;
    typedef shared_ptr<const ParallacticRotation>   ConstPtr;

    ParallacticRotation(const Expr<Vector<3> >::ConstPtr &target,
        const AntennaField::ConstPtr &field);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &target) const;

private:
    AntennaField::ConstPtr  itsField;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
