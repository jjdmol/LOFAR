//# LMN.h: LMN-coordinates of a direction on the sky.
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

#ifndef LOFAR_BBSKERNEL_EXPR_LMN_H
#define LOFAR_BBSKERNEL_EXPR_LMN_H

// \file
// LMN-coordinates of a direction on the sky.

#include <BBSKernel/Expr/BasicExpr.h>

#include <measures/Measures/MDirection.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class LMN: public BasicUnaryExpr<Vector<2>, Vector<3> >
{
public:
    typedef shared_ptr<LMN>         Ptr;
    typedef shared_ptr<const LMN>   ConstPtr;

    LMN(const casa::MDirection &reference,
        const Expr<Vector<2> >::ConstPtr &direction);

protected:
    virtual const Vector<3>::View evaluateImpl(const Grid &grid,
        const Vector<2>::View &direction) const;

private:
    casa::MDirection    itsPhaseReference;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
