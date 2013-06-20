//# StokesRM.h: Stokes vector with Q and U parameterized by polarizaed fraction,
//# polarization angle, and rotation measure.
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

#ifndef LOFAR_BBSKERNEL_EXPR_STOKESRM_H
#define LOFAR_BBSKERNEL_EXPR_STOKESRM_H

// \file
// Stokes vector with Q and U parameterized by polarized fraction, polarization
// angle, and rotation measure.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class StokesRM: public BasicExpr5<Scalar, Scalar, Scalar, Scalar, Scalar,
    Vector<4> >
{
public:
    typedef shared_ptr<StokesRM>        Ptr;
    typedef shared_ptr<const StokesRM>  ConstPtr;

    StokesRM(const Expr<Scalar>::ConstPtr &stokesI,
        const Expr<Scalar>::ConstPtr &stokesV,
        const Expr<Scalar>::ConstPtr &polFraction,
        const Expr<Scalar>::ConstPtr &polAngle0,
        const Expr<Scalar>::ConstPtr &rm);

protected:
    virtual const Vector<4>::View evaluateImpl(const Grid &grid,
        const Scalar::View &stokesI,
        const Scalar::View &stokesV,
        const Scalar::View &polFraction,
        const Scalar::View &polAngle0,
        const Scalar::View &rm) const;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
