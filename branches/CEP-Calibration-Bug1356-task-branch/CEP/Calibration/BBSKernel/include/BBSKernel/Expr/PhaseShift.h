//# PhaseShift.h: Phase delay due to baseline geometry with respect to a
//#     direction on the sky.
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

#ifndef LOFAR_BBSKERNEL_EXPR_PHASESHIFT_H
#define LOFAR_BBSKERNEL_EXPR_PHASESHIFT_H

// \file
// Phase delay due to baseline geometry with respect to a direction on the sky.

#include <BBSKernel/Expr/BasicExpr.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PhaseShift: public BasicBinaryExpr<Vector<2>, Vector<2>, Scalar>
{
public:
    typedef shared_ptr<PhaseShift>          Ptr;
    typedef shared_ptr<const PhaseShift>    ConstPtr;

    PhaseShift(const Expr<Vector<2> >::ConstPtr &lhs,
        const Expr<Vector<2> >::ConstPtr &rhs);

protected:
    virtual const Scalar::View evaluateImpl(const Request &request,
        const Vector<2>::View &lhs, const Vector<2>::View &rhs) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
