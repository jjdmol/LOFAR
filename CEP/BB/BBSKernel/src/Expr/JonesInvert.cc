//# JonesInvert.cc: The inverse of a Jones matrix expression.
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

#include <lofar_config.h>

#include <BBSKernel/Expr/JonesInvert.h>
#include <BBSKernel/Expr/Request.h>
#include <BBSKernel/Expr/Matrix.h>
#include <BBSKernel/Expr/MatrixTmp.h>
#include <BBSKernel/Expr/PValueIterator.h>

#include <Common/lofar_iomanip.h>

// Inverse of a 2x2 matrix:
//
//        (a b)                      ( d -b)
// If A = (   ) then inverse(A)   =  (     ) / (ad-bc)
//        (c d)                      (-c  a)
//
// Note that:
//            (conj(a) conj(c))
//  conj(A) = (               )
//            (conj(b) conj(d))
//
// so  inverse(conj(A)) = conj(inverse(A))
//
// Also note that conj(AB) = conj(B)conj(A)

namespace LOFAR
{
namespace BBS
{

JonesInvert::JonesInvert (const JonesExpr& expr)
    : itsExpr(expr)
{
    addChild(itsExpr);
}

JonesInvert::~JonesInvert()
{
}

JonesResult JonesInvert::getJResult (const Request& request)
{
    // Create the result object.
    JonesResult result;
    result.init();

    Result& result11 = result.result11();
    Result& result12 = result.result12();
    Result& result21 = result.result21();
    Result& result22 = result.result22();

    // Calculate the children.
    JonesResult buf;
    const JonesResult& res = itsExpr.getResultSynced (request, buf);
    const Result& r11 = res.getResult11();
    const Result& r12 = res.getResult12();
    const Result& r21 = res.getResult21();
    const Result& r22 = res.getResult22();
    const Matrix& mr11 = r11.getValue();
    const Matrix& mr12 = r12.getValue();
    const Matrix& mr21 = r21.getValue();
    const Matrix& mr22 = r22.getValue();
    Matrix t(1. / (mr11 * mr22 - mr12 * mr21));
    result11.setValue (mr22 * t);
    result12.setValue (mr12 * -t);
    result21.setValue (mr21 * -t);
    result22.setValue (mr11 * t);

    // Compute the perturbed values.
    enum PValues
    { PV_J11, PV_J12, PV_J21, PV_J22, N_PValues };

    const Result *pvSet[N_PValues] = {&r11, &r12, &r21, &r22};
    PValueSetIterator<N_PValues> pvIter(pvSet);

    while(!pvIter.atEnd())
    {
        const Matrix &mr11 = pvIter.value(PV_J11);
        const Matrix &mr12 = pvIter.value(PV_J12);
        const Matrix &mr21 = pvIter.value(PV_J21);
        const Matrix &mr22 = pvIter.value(PV_J22);

        Matrix t(1. / (mr11 * mr22 - mr12 * mr21));

        result11.setPerturbedValue(pvIter.key(), mr22 * t);
        result12.setPerturbedValue(pvIter.key(), mr12 * -t);
        result21.setPerturbedValue(pvIter.key(), mr21 * -t);
        result22.setPerturbedValue(pvIter.key(), mr11 * t);

        pvIter.next();
    }

    return result;
}

} // namespace BBS
} // namespace LOFAR
