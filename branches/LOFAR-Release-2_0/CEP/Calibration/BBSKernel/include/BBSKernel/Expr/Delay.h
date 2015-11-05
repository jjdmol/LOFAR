//# Delay.h: Phase shift corresponding to a (differential) delay in seconds.
//#
//# Copyright (C) 2010
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

#ifndef LOFAR_BBSKERNEL_EXPR_DELAY_H
#define LOFAR_BBSKERNEL_EXPR_DELAY_H

// \file
// Phase shift corresponding to a (differential) delay in seconds.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class Delay: public BasicUnaryExpr<Scalar, Scalar>
{
public:
    typedef shared_ptr<Delay>       Ptr;
    typedef shared_ptr<const Delay> ConstPtr;

    Delay(const Expr<Scalar>::ConstPtr &delay);

protected:
    virtual const Scalar::View evaluateImpl(const Grid &grid,
        const Scalar::View &delay) const;
};

// @}

} //# namespace BBS
} //# namespace LOFAR

#endif
