//# PointCoherence.h: Spatial coherence function of a point source.
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

#ifndef LOFAR_BBSKERNEL_EXPR_POINTCOHERENCE_H
#define LOFAR_BBSKERNEL_EXPR_POINTCOHERENCE_H

// \file
// Spatial coherence function of a point source.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/ExprResult.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

class PointCoherence: public Expr1<Vector<4>, JonesMatrix>
{
public:
    typedef shared_ptr<PointCoherence>          Ptr;
    typedef shared_ptr<const PointCoherence>    ConstPtr;

    PointCoherence(const Expr<Vector<4> >::ConstPtr &stokes);

private:
    // Compute a result for the given request.
    virtual const JonesMatrix::proxy evaluateImpl(const Request &request,
        const Vector<4>::proxy &stokes) const;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
