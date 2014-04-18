//# ElevationCut.h: Trivial beam model that is equal to 1.0 above the elevation
//# cut-off and 0.0 otherwise.
//#
//# Copyright (C) 2012
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

#ifndef LOFAR_BBSKERNEL_EXPR_ELEVATIONCUT_H
#define LOFAR_BBSKERNEL_EXPR_ELEVATIONCUT_H

// \file
// Trivial beam model that is equal to 1.0 above the elevation cut-off and 0.0
// otherwise.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup PACKAGE_NAME
// @{

class ElevationCut: public BasicUnaryExpr<Vector<2>, JonesMatrix>
{
public:
    typedef shared_ptr<ElevationCut>        Ptr;
    typedef shared_ptr<const ElevationCut>  ConstPtr;

    ElevationCut(const Expr<Vector<2> >::ConstPtr &azel, double threshold);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<2>::View &azel) const;

private:
    double  itsThreshold;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
