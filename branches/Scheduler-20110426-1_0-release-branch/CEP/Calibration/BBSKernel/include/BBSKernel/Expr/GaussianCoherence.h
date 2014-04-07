//# GaussianCoherence.h: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
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

#ifndef LOFAR_BBSKERNEL_EXPR_GAUSSIANCOHERENCE_H
#define LOFAR_BBSKERNEL_EXPR_GAUSSIANCOHERENCE_H

// \file
// Spatial coherence function of an elliptical gaussian source.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class GaussianCoherence: public BasicExpr5<Vector<4>, Vector<2>, Scalar,
    Vector<3>, Vector<3>, JonesMatrix>
{
public:
    typedef shared_ptr<GaussianCoherence>       Ptr;
    typedef shared_ptr<const GaussianCoherence> ConstPtr;

    GaussianCoherence(const Expr<Vector<4> >::ConstPtr stokes,
        const Expr<Vector<2> >::ConstPtr dimensions,
        const Expr<Scalar>::ConstPtr orientation,
        const Expr<Vector<3> >::ConstPtr &uvwA,
        const Expr<Vector<3> >::ConstPtr &uvwB);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<4>::View &stokes, const Vector<2>::View &dimensions,
        const Scalar::View &orientation, const Vector<3>::View &uvwA,
        const Vector<3>::View &uvwB) const;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
