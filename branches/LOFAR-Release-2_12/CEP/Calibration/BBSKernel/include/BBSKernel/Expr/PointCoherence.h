//# PointCoherence.h: Spatial coherence function of a point source.
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_BBSKERNEL_EXPR_POINTCOHERENCE_H
#define LOFAR_BBSKERNEL_EXPR_POINTCOHERENCE_H

// \file
// Spatial coherence function of a point source.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PointCoherence: public BasicUnaryExpr<Vector<4>, JonesMatrix>
{
public:
    typedef shared_ptr<PointCoherence>          Ptr;
    typedef shared_ptr<const PointCoherence>    ConstPtr;

    PointCoherence(const Expr<Vector<4> >::ConstPtr &stokes);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<4>::View &stokes) const;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
