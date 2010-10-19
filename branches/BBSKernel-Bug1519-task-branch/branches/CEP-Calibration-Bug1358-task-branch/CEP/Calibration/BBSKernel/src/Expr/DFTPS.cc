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
#include <BBSKernel/Expr/MatrixTmp.h>
//#include <BBSKernel/Expr/PValueIterator.h>
#include <Common/LofarLogger.h>

#include <casa/BasicSL/Constants.h>

using namespace casa;

namespace LOFAR
{
namespace BBS
{

DFTPS::DFTPS(const StatUVW::ConstPointer &uvw)
    : ExprStatic<DFTPS::N_Arguments>(),
      itsUVW(uvw)
{
}

Shape DFTPS::shape(const ExprValueSet (&arguments)[DFTPS::N_Arguments]) const
{
    DBGASSERT(arguments[0].shape() == Shape(3));
    return Shape(2);
}

void DFTPS::evaluateImpl(const Request &request,
    const ExprValue (&arguments)[DFTPS::N_Arguments], ExprValue &result) const
{
    // It is assumed that the channels are regularly spaced, i.e. the channel
    // frequency can be written as f0 + k * df where f0 is the frequency of
    // the first channel and k is an integer. Under this assumption, the phase
    // shift can be expressed as:
    //
    // Let (u . l) = u * l + v * m + w * (n - 1.0), then:
    //
    // shift = exp(i * 2.0 * pi * (u . l) * (f0 + k * df) / c)
    //       = exp(i * (2.0 * pi / c) * (u . l) * f0)
    //         * exp(i * (2.0 * pi / c) * (u . l) * df) ^ k
    //
    // The StationShift node only computes the two exponential terms.
    // Computing the phase shift for each channel is performed by the node that
    // computes the phase shift for a baseline (by combining the result of two
    // StationShift nodes).

    // Check precondition (frequency axis must be regular).
    const Grid &reqGrid = request.getGrid();
    ASSERT(dynamic_cast<RegularAxis*>(reqGrid[FREQ].get()) != 0);

    // Allocate the result.
//    ValueSet::Ptr result(new ValueSet(2));
//    ResultVec resultVec(2);
    bool calcDelta = request[FREQ]->size() > 1;

    // Calculate 2.0 * pi / lambda0, where lambda0 = c / f0.
    double df = reqGrid[FREQ]->width(0);
    double f0 = reqGrid[FREQ]->center(0);
    Matrix wavel0(C::_2pi * f0 / C::c);
    Matrix dwavel(df / f0);

    // Get the UVW coordinates.
    const ExprValueSet resU = itsUVW->getU(request);
    const ExprValueSet resV = itsUVW->getV(request);
    const ExprValueSet resW = itsUVW->getW(request);
    const Matrix &u = resU();
    const Matrix &v = resV();
    const Matrix &w = resW();

    // Calculate the DFT contribution for this (station, direction) combination.
    const Matrix &l = arguments[LMN](0);
    const Matrix &m = arguments[LMN](1);
    const Matrix &n = arguments[LMN](2);

    // Note that both UVW and LMN are required to either scalar or variable in
    // time.
    ASSERT(u.nx() == 1 && v.nx() == 1 && w.nx() == 1);
    ASSERT(l.nx() == 1 && m.nx() == 1 && n.nx() == 1);

    Matrix phase0 = (u * l + v * m + w * (n - 1.0)) * wavel0;
    result.assign(0, tocomplex(cos(phase0), sin(phase0)));

//    resultVec[0].setValue(tocomplex(cos(phase0), sin(phase0)));

    if(calcDelta)
    {
        phase0 *= dwavel;
        result.assign(1, tocomplex(cos(phase0), sin(phase0)));
//        resultVec[1].setValue(tocomplex(cos(phase0), sin(phase0)));
    }

//    // Compute the perturbed values.
//    enum PValues
//    {
//        PV_U, PV_V, PV_W, PV_L, PV_M, PV_N, N_PValues
//    };
//
//    const Result *pvSet[N_PValues] =
//        {&resU, &resV, &resW, &resL, &resM, &resN};
//    PValueSetIterator<N_PValues> pvIter(pvSet);
//
//    while(!pvIter.atEnd())
//    {
//        const Matrix &pvU = pvIter.value(PV_U);
//        const Matrix &pvV = pvIter.value(PV_V);
//        const Matrix &pvW = pvIter.value(PV_W);
//        const Matrix &pvL = pvIter.value(PV_L);
//        const Matrix &pvM = pvIter.value(PV_M);
//        const Matrix &pvN = pvIter.value(PV_N);
//
//        phase0 = (pvU * pvL + pvV * pvM + pvW * (pvN - 1.0)) * wavel0;
//        resultVec[0].setPerturbedValue(pvIter.key(), tocomplex(cos(phase0),
//            sin(phase0)));
//
//        if(calcDelta)
//        {
//            phase0 *= dwavel;
//            resultVec[1].setPerturbedValue(pvIter.key(), tocomplex(cos(phase0),
//                sin(phase0)));
//        }

//        pvIter.next();
//    }

//    return result;
}


#if 0
string DFTPS::getLabel()
{
    return string("DFTPS\\nStation part of baseline phase\\n");
}
#endif

} // namespace BBS
} // namespace LOFAR
