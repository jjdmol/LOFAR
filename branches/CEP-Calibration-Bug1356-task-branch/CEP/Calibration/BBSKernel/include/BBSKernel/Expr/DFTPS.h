//# StationShift.h: Station part of baseline phase shift.
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_BBS_EXPR_DFTPS_H
#define LOFAR_BBS_EXPR_DFTPS_H

// \file
// Station part of baseline phase shift.

#include <BBSKernel/Expr/Expr.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// DFTPS computes the station part of the phase related to a direction (l, m, n)
// on the sky.
//
// It is assumed that the frequency axis of the request is regularly spaced,
// i.e. the frequency can be written as f0 + k * df where f0 is the frequency of
// the first channel and k is an integer. Under this assumption, the phase shift
// can be expressed as:
//
// Let (u . l) = u * l + v * m + w * (n - 1.0), then:
//
// shift = exp(i * 2.0 * pi * (u . l) * (f0 + k * df) / c)
//       = exp(i * (2.0 * pi / c) * (u . l) * f0)
//         * exp(i * (2.0 * pi / c) * (u . l) * df) ^ k
//
// DFTPS only computes the two exponential terms. PhaseShift computes the phase
// shift for a baseline for each frequency by combining the results of two
// DFTPS nodes and applying the above equation.
class DFTPS: public BasicBinaryExpr<Vector<3>, Vector<3>, Vector<2> >
{
public:
    typedef shared_ptr<DFTPS>       Ptr;
    typedef shared_ptr<const DFTPS> ConstPtr;

    DFTPS(const Expr<Vector<3> >::ConstPtr &uvw,
        const Expr<Vector<3> >::ConstPtr &lmn);

private:
    virtual const Vector<2>::view evaluateImpl(const Request &request,
        const Vector<3>::view &uvw, const Vector<3>::view &lmn) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
