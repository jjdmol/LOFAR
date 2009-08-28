//# LMN.h: LMN-coordinates of a direction on the sky.
//#
//# Copyright (C) 2005
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

private:
    virtual const Vector<3>::View evaluateImpl(const Request &request,
        const Vector<2>::View &direction) const;

    casa::MDirection    itsPhaseReference;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
