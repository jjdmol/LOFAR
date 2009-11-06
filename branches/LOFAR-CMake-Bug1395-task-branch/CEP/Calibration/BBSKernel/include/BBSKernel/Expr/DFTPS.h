//# StationShift.h: Station part of baseline phase shift.
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

#ifndef EXPR_DFTPS_H
#define EXPR_DFTPS_H

// \file
// Station part of baseline phase shift.

#include <BBSKernel/Expr/Expr.h>
#include <BBSKernel/Expr/StatUVW.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
namespace BBS
{

// \ingroup Expr
// @{

// DFTPS computes the station part of the phase related to a direction (l, m, n)
// on the sky.
//
// It is assumed that the frequency axis of the request is regularly spaced,
// i.e. the frequency can be written as f0 + k * df where f0 is the frequency of
// the first sample and k is an integer. Under this assumption, the phase shift
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
class DFTPS: public ExprRep
{
public:
    DFTPS(const StatUVW::ConstPointer &uvw, const Expr &lmn);
    virtual ~DFTPS();

    virtual ResultVec getResultVec(const Request &request);

private:
    StatUVW::ConstPointer    itsUVW;
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
