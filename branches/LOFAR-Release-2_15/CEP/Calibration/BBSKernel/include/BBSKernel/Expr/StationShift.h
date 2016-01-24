//# StationShift.h: Station part of baseline phase shift for a given direction.
//#
//# Copyright (C) 2002
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

#ifndef LOFAR_BBSKERNEL_EXPR_STATIONSHIFT_H
#define LOFAR_BBSKERNEL_EXPR_STATIONSHIFT_H

// \file
// Station part of baseline phase shift for a given direction.

#include <BBSKernel/Expr/BasicExpr.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \addtogroup Expr
// @{

// StationShift computes the station part of the phase related to a direction
// (l, m, n) on the sky.
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
// StationShift only computes the two exponential terms. PhaseShift computes the
// phase shift for a baseline for each frequency by combining the results of two
// StationShift expressions according to the above equation.
class StationShift: public BasicBinaryExpr<Vector<3>, Vector<3>, Vector<2> >
{
public:
    typedef shared_ptr<StationShift>        Ptr;
    typedef shared_ptr<const StationShift>  ConstPtr;

    StationShift(const Expr<Vector<3> >::ConstPtr &uvw,
        const Expr<Vector<3> >::ConstPtr &lmn);

protected:
    virtual const Vector<2>::View evaluateImpl(const Grid &grid,
        const Vector<3>::View &uvw, const Vector<3>::View &lmn) const;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
