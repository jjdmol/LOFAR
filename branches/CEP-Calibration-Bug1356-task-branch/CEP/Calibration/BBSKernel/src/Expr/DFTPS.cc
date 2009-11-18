//# DFTPS.cc: Station part of baseline phase shift.
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

#include <lofar_config.h>

#include <BBSKernel/Expr/DFTPS.h>
#include <BBSKernel/Expr/StatUVW.h>
#include <BBSKernel/Expr/LMN.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/ExprResult.h>
#include <Common/LofarLogger.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

DFTPS::DFTPS(const Expr<Vector<3> >::ConstPtr &uvw,
    const Expr<Vector<3> >::ConstPtr &lmn)
    :   BasicBinaryExpr<Vector<3>, Vector<3>, Vector<2> >(uvw, lmn)
{
}

const Vector<2>::View DFTPS::evaluateImpl(const Request &request,
    const Vector<3>::View &uvw, const Vector<3>::View &lmn) const
{
    // Check precondition (frequency axis must be regular).
    ASSERT(dynamic_cast<RegularAxis*>(request[FREQ].get()) != 0);

    // Calculate 2.0 * pi / lambda0, where lambda0 = c / f0.
    const double lambda0 = C::_2pi * request[FREQ]->center(0) / C::c;
    const double dlambda = request[FREQ]->width(0) / request[FREQ]->center(0);

    // NOTE: Both UVW and LMN are vectors are required to be either scalar or
    // variable in time.
    ASSERT(uvw(0).nx() == 1 && uvw(1).nx() == 1 && uvw(2).nx() == 1);
    ASSERT(lmn(0).nx() == 1 && lmn(1).nx() == 1 && lmn(2).nx() == 1);

    // Create the result.
    Vector<2>::View result;

    // Calculate the DFT contribution for this (station, direction) combination.
    Matrix phase0 = (uvw(0) * lmn(0) + uvw(1) * lmn(1) + uvw(2)
        * (lmn(2) - 1.0)) * lambda0;
    result.assign(0, tocomplex(cos(phase0), sin(phase0)));

    // If the request contains multiple frequencies then a "step" term is
    // computed to be able to efficiently compute the phase shift at
    // different frequencies in the PhaseShift node (see explanation in the
    // DFTPS header file).
    if(request[FREQ]->size() > 1)
    {
        phase0 *= dlambda;
        result.assign(1, tocomplex(cos(phase0), sin(phase0)));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
