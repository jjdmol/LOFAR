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

#include <BBSKernel/Expr/BasicExpr.h>
#include <BBSKernel/Expr/PointSource.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

class PointCoherence: public BasicBinaryExpr<Vector<4>, Scalar, JonesMatrix>
{
public:
    typedef shared_ptr<PointCoherence>          Ptr;
    typedef shared_ptr<const PointCoherence>    ConstPtr;

    PointCoherence(const PointSource::ConstPtr &source);

private:
    virtual const JonesMatrix::View evaluateImpl(const Request &request,
        const Vector<4>::View &stokes, const Scalar::View &spectral) const;
};

class PointCoherenceSimple: public BasicUnaryExpr<Vector<4>, JonesMatrix>
{
public:
    typedef shared_ptr<PointCoherenceSimple>        Ptr;
    typedef shared_ptr<const PointCoherenceSimple>  ConstPtr;

    PointCoherenceSimple(const Expr<Vector<4> >::ConstPtr &stokes);

private:
    virtual const JonesMatrix::View evaluateImpl(const Request &request,
        const Vector<4>::View &stokes) const;
};

// @}

} // namespace BBS
} // namespace LOFAR
#endif
