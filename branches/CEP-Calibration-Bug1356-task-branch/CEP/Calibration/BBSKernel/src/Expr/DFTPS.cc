//# DFTPS.cc: Station part of baseline phase shift.
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

DFTPS::DFTPS(const StatUVW::ConstPointer &uvw,
    const Expr<Vector<3> >::ConstPtr &lmn)
    :   Expr1<Vector<3>, Vector<2> >(lmn),
        itsUVW(uvw)
{
}

const Vector<2>::proxy DFTPS::evaluateImpl(const Request &request,
        const Vector<3>::proxy &lmn) const
{
    // If the request contains multiple frequencies then a "step" term is
    // computed to be able to efficiently compute the phase shift at
    // different frequencies in the PhaseShift node (see explanation in the
    // DFTPS header file).
    const bool calcDelta = request[FREQ]->size() > 1;

    // Check precondition (frequency axis must be regular).
    const Grid &reqGrid = request.getGrid();
    ASSERT(dynamic_cast<RegularAxis*>(reqGrid[FREQ].get()) != 0);

    // Calculate 2.0 * pi / lambda0, where lambda0 = c / f0.
    const double wavel0 = C::_2pi * reqGrid[FREQ]->center(0) / C::c;
    const double dwavel = reqGrid[FREQ]->width(0) / reqGrid[FREQ]->center(0);

    // Get the UVW coordinates.
    // TODO: Re-implement the UVW node as a regular Expr<> derived class that
    // returns a Vector<3> instead of 3 separate Scalar's.
    Scalar resU = itsUVW->getU(request);
    Scalar resV = itsUVW->getV(request);
    Scalar resW = itsUVW->getW(request);
    const Matrix &u = resU.getFieldSet().value();
    const Matrix &v = resV.getFieldSet().value();
    const Matrix &w = resW.getFieldSet().value();

    // NOTE: Both UVW and LMN are vectors are required to be either scalar or
    // variable in time.
    ASSERT(u.nx() == 1 && v.nx() == 1 && w.nx() == 1);
    ASSERT(lmn(0).nx() == 1 && lmn(1).nx() == 1 && lmn(2).nx() == 1);

    // Create the result.
    Vector<2>::proxy result;

    // Calculate the DFT contribution for this (station, direction) combination.
    Matrix phase0 = (u * lmn(0) + v * lmn(1) + w * (lmn(2) - 1.0)) * wavel0;
    result.assign(0, tocomplex(cos(phase0), sin(phase0)));

    if(calcDelta)
    {
        phase0 *= dwavel;
        result.assign(1, tocomplex(cos(phase0), sin(phase0)));
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
