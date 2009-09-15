//# GaussianCoherence.h: Spatial coherence function of an elliptical gaussian
//# source.
//#
//# Copyright (C) 2008
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

#ifndef EXPR_GAUSSIANCOHERENCE_H
#define EXPR_GAUSSIANCOHERENCE_H

// \file
// Spatial coherence function of an elliptical gaussian source.

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Expr/GaussianSource.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class GaussianCoherence: public BasicExpr6<Vector<4>, Scalar, Vector<2>, Scalar,
    Vector<3>, Vector<3>, JonesMatrix>
{
public:
    typedef shared_ptr<GaussianCoherence>       Ptr;
    typedef shared_ptr<const GaussianCoherence> ConstPtr;

    GaussianCoherence(const GaussianSource::ConstPtr &source,
        const Expr<Vector<3> >::ConstPtr &uvwA,
        const Expr<Vector<3> >::ConstPtr &uvwB);

private:
    virtual const JonesMatrix::View evaluateImpl(const Request &request,
        const Vector<4>::View &stokes, const Scalar::View &spectral,
        const Vector<2>::View &dimensions, const Scalar::View &orientation,
        const Vector<3>::View &uvwA, const Vector<3>::View &uvwB) const;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
