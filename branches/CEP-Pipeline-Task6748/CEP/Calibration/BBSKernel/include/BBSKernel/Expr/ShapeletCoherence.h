//# ShapeletCoherence.h: Spatial coherence function of a source modelled as a
//# shapelet basis expansion.
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
//# $Id: ShapeletCoherence.h 14789 2010-01-13 12:39:15Z zwieten $

#ifndef LOFAR_BBSKERNEL_EXPR_SHAPELETCOHERENCE_H
#define LOFAR_BBSKERNEL_EXPR_SHAPELETCOHERENCE_H

// \file
// Spatial coherence function of a source modelled as a shapelet basis
// expansion.

#include <BBSKernel/Expr/BasicExpr.h>
#include <Common/lofar_complex.h>

#include <casa/Arrays.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class ShapeletCoherence: public BasicTernaryExpr<Vector<4>, Vector<3>,
    Vector<3>, JonesMatrix>
{
public:
    typedef shared_ptr<ShapeletCoherence>       Ptr;
    typedef shared_ptr<const ShapeletCoherence> ConstPtr;

    ShapeletCoherence(const Expr<Vector<4> >::ConstPtr stokes, double scaleI,
        const casa::Array<double> &coeffI,
        const Expr<Vector<3> >::ConstPtr &uvwA,
        const Expr<Vector<3> >::ConstPtr &uvwB);

protected:
    virtual const JonesMatrix::View evaluateImpl(const Grid &grid,
        const Vector<4>::View &stokes, const Vector<3>::View &uvwA,
        const Vector<3>::View &uvwB) const;

    virtual const JonesMatrix::View evaluateImplI(const Grid &grid,
        const Vector<4>::View &stokes, double scaleI,
        const casa::Array<double> &coeffI, const Vector<3>::View &uvwA,
        const Vector<3>::View &uvwB) const;

    double              itsShapeletScaleI_;
    casa::Array<double> itsShapeletCoeffI_;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
